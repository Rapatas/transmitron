#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <string_view>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <wx/artprov.h>

#include "Common/Url.hpp"
#include "Common/Log.hpp"
#include "Snippets.hpp"

namespace fs = std::filesystem;
using namespace Transmitron::Models;
using namespace Common;

Snippets::Snippets()
{
  mLogger = Common::Log::create("Models::Snippets");

  const auto id = getNextId();
  Node root {
    0,
    "_",
    {},
    Node::Type::Folder,
    {},
    {},
    true
  };

  mNodes.insert({id, std::move(root)});
}

MQTT::Message Snippets::getMessage(wxDataViewItem item) const
{
  if (!item.IsOk())
  {
    return {};
  }

  const auto id = toId(item);
  const auto &node = mNodes.at(id);
  return node.message;
}

std::string Snippets::getName(wxDataViewItem item) const
{
  if (!item.IsOk())
  {
    return {};
  }

  const auto id = toId(item);
  const auto &node = mNodes.at(id);
  return node.name;
}

wxDataViewItem Snippets::getRootItem()
{
  return toItem(0);
}

bool Snippets::load(const std::string &connectionDir)
{
  if (connectionDir.empty())
  {
    mLogger->warn("No directory provided");
    return false;
  }

  mSnippetsDir = connectionDir + "/snippets";

  bool exists = fs::exists(mSnippetsDir);
  bool isDir = fs::is_directory(mSnippetsDir);

  if (exists && !isDir && !fs::remove(mSnippetsDir))
  {
    mLogger->warn("Could not remove file {}", mSnippetsDir);
    return false;
  }

  if (!exists && !fs::create_directory(mSnippetsDir))
  {
    mLogger->warn(
      "Could not create snippets directory: {}",
      mSnippetsDir
    );
    return false;
  }

  mNodes.at(0).encoded = mSnippetsDir;

  loadDirectoryRecursive(mSnippetsDir, std::numeric_limits<Node::Id_t>::max());

  return true;
}

wxDataViewItem Snippets::createFolder(
  wxDataViewItem parentItem
) {
  const constexpr std::string_view NewName{"New Folder"};

  auto parentId = toId(parentItem);
  Node &parentNode = mNodes.at(parentId);
  if (parentNode.type != Node::Type::Folder)
  {
    return wxDataViewItem(nullptr);
  }

  // Find a unique name.
  std::string uniqueName{NewName};
  size_t postfix = 0;
  while (hasChildNamed(parentItem, uniqueName)) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", NewName, postfix);
  }

  mLogger->info(
    "Creating folder '{}' under [{}]'{}'",
    uniqueName,
    parentId,
    parentNode.name
  );

  const std::string encoded = Url::encode(uniqueName);

  const std::string path = fmt::format(
    "{}/{}",
    parentNode.encoded,
    encoded
  );

  Node newNode {
    parentId,
    uniqueName,
    encoded,
    Node::Type::Folder,
    {},
    {},
    false,
  };
  const auto newId = getNextId();
  mNodes.insert({newId, std::move(newNode)});
  mNodes.at(parentId).children.push_back(newId);

  mNodes.at(parentId).saved = false;
  save(newId);

  const auto item = toItem(newId);
  ItemAdded(parentItem, item);

  return item;
}

wxDataViewItem Snippets::createSnippet(
  wxDataViewItem parentItem,
  const MQTT::Message &message
) {
  const constexpr std::string_view NewName{"New Snippet"};

  auto parentId = toId(parentItem);
  Node &parentNode = mNodes.at(parentId);
  if (parentNode.type != Node::Type::Folder)
  {
    return wxDataViewItem(nullptr);
  }

  // Find a unique name.
  std::string uniqueName{NewName};
  size_t postfix = 0;
  while (hasChildNamed(parentItem, uniqueName)) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", NewName, postfix);
  }

  mLogger->info(
    "Creating snippet '{}' under [{}]'{}'",
    uniqueName,
    parentId,
    parentNode.name
  );

  const std::string encoded = Url::encode(uniqueName);

  Node newNode {
    parentId,
    uniqueName,
    encoded,
    Node::Type::Snippet,
    {},
    message,
    false,
  };
  const auto newId = getNextId();
  mNodes.insert({newId, std::move(newNode)});
  mNodes.at(parentId).children.push_back(newId);

  mNodes.at(parentId).saved = false;
  save(newId);

  const auto item = toItem(newId);
  ItemAdded(parentItem, item);

  return item;
}

wxDataViewItem Snippets::insert(
  const std::string &name,
  const MQTT::Message &message,
  wxDataViewItem parentItem
) {
  const auto parentId = toId(parentItem);
  const auto &parentNode = mNodes.at(parentId);

  if (parentNode.type != Node::Type::Folder)
  {
    return wxDataViewItem(nullptr);
  }

  if (hasChildNamed(parentItem, name))
  {
    return wxDataViewItem(nullptr);
  }

  mLogger->info(
    "Creating snippet '{}' under [{}]'{}'",
    name,
    parentId,
    parentNode.name
  );

  const std::string encoded = Url::encode(name);

  const auto parentPath = getNodePath(parentId);
  const std::string path = fmt::format(
    "{}/{}",
    parentPath,
    encoded
  );

  Node newNode {
    parentId,
    name,
    encoded,
    Node::Type::Snippet,
    {},
    message,
    false,
  };
  const auto newId = getNextId();
  mNodes.insert({newId, std::move(newNode)});
  mNodes.at(parentId).children.push_back(newId);

  save(newId);

  const auto item = toItem(newId);
  ItemAdded(parentItem, item);

  return item;
}

wxDataViewItem Snippets::replace(
  wxDataViewItem item,
  const MQTT::Message &message
) {
  const auto id = toId(item);
  auto &node = mNodes.at(id);
  if (node.type != Node::Type::Snippet)
  {
    mLogger->warn("Can only replace snippets");
    return wxDataViewItem(nullptr);
  }

  const auto path = getNodePath(id);
  std::ofstream output(path);
  if (!output.is_open())
  {
    mLogger->warn("Could not open '{}'", path);
    return wxDataViewItem(nullptr);
  }

  output << MQTT::Message::toJson(message);

  node.message = message;
  return item;
}

bool Snippets::remove(wxDataViewItem item)
{
  const auto id = toId(item);
  auto &node = mNodes.at(id);
  const auto parent = GetParent(item);

  std::error_code ec;
  const auto path = getNodePath(id);
  fs::remove_all(path, ec);
  if (ec)
  {
    mLogger->error("Could not delete '{}': {}", node.name, ec.message());
    return false;
  }

  auto &children = mNodes.at(toId(parent)).children;
  auto removeIt = std::remove(
    std::begin(children),
    std::end(children),
    id
  );
  children.erase(removeIt);
  mNodes.erase(id);
  ItemDeleted(parent, item);

  return true;
}

wxDataViewItem Snippets::moveBefore(
  wxDataViewItem item,
  wxDataViewItem sibling
) {
  const auto parent = GetParent(sibling);
  wxDataViewItemArray children;
  GetChildren(parent, children);

  size_t index = 0;
  for (size_t i = 0; i != children.size(); ++i)
  {
    if (children[i].GetID() == sibling.GetID())
    {
      index = i;
      break;
    }
  }

  return moveAtIndex(item, parent, index);
}

wxDataViewItem Snippets::moveLast(
  wxDataViewItem item,
  wxDataViewItem parent
) {
  wxDataViewItemArray children;
  const auto count = GetChildren(parent, children);
  return moveAtIndex(item, parent, count);
}

wxDataViewItem Snippets::moveAfter(
  wxDataViewItem item,
  wxDataViewItem sibling
) {
  const auto parent = GetParent(sibling);
  wxDataViewItemArray children;
  GetChildren(parent, children);

  size_t index = 0;
  for (size_t i = 0; i != children.size(); ++i)
  {
    if (children[i].GetID() == sibling.GetID())
    {
      index = i + 1;
      break;
    }
  }

  return moveAtIndex(item, parent, index);
}

wxDataViewItem Snippets::moveInside(
  wxDataViewItem item,
  wxDataViewItem parent
) {
  return moveAtIndex(item, parent, 0);
}

wxDataViewItem Snippets::moveAtIndex(
  wxDataViewItem item,
  wxDataViewItem parent,
  size_t index
) {

  if (!moveCheck(item, parent, index))
  {
    return wxDataViewItem(nullptr);
  }

  const auto nodeId = toId(item);
  const auto &node = mNodes.at(nodeId);
  const auto newParentId = toId(parent);
  const auto oldParentId = node.parent;

  if (!moveFile(nodeId, newParentId))
  {
    return wxDataViewItem(nullptr);
  }

  if (oldParentId != newParentId)
  {
    mLogger->info(
      "Moving {} in {} at index {}",
      nodeId,
      newParentId,
      index
    );
    moveUnderNewParent(nodeId, newParentId, index);
  }
  else
  {
    mLogger->info(
      "Moving {} at index {}",
      nodeId,
      newParentId,
      index
    );
    moveUnderSameParent(nodeId, newParentId, index);
  }

  ItemDeleted(toItem(oldParentId), item);
  ItemAdded(parent, item);
  for (const auto &child : node.children)
  {
    ItemAdded(item, toItem(child));
  }

  mNodes.at(newParentId).saved = false;
  save(newParentId);

  return item;
}

void Snippets::loadDirectoryRecursive(
  const std::filesystem::path &path,
  Node::Id_t parentId
) {
  const bool isRoot = parentId == std::numeric_limits<Node::Id_t>::max();
  const auto currentId = isRoot ? 0 : getNextId();
  std::string decoded = "_";

  if (!isRoot)
  {
    try
    {
      decoded = Url::decode(path.stem().string());
    }
    catch (std::runtime_error &e)
    {
      mLogger->error(
        "Could not decode '{}': {}",
        path.string(),
        e.what()
      );
      return;
    }

    Node newNode {
      parentId,
      decoded,
      path.filename().string(),
      Node::Type::Folder,
      {},
      {},
      true,
    };

    mNodes.insert({currentId, std::move(newNode)});
    mNodes.at(parentId).children.push_back(currentId);
  }

  for (const auto &entry : fs::directory_iterator(path))
  {
    if (entry.path().stem() == ".index") { continue; }

    if (entry.status().type() == fs::file_type::directory)
    {
      loadDirectoryRecursive(entry.path(), currentId);
    }
    else
    {
      loadSnippet(entry.path(), currentId);
    }
  }

  if (mNodes.at(currentId).children.empty())
  {
    return;
  }

  const bool read = indexFileRead(path, currentId);
  if (!read)
  {
    indexFileWrite(currentId);
  }
}

void Snippets::loadSnippet(
  const std::filesystem::path &path,
  Node::Id_t parentId
) {
  std::string decoded;
  try
  {
    decoded = Url::decode(path.stem().string());
  }
  catch (std::runtime_error &e)
  {
    mLogger->error(
      "Could not decode '{}': {}",
      path.string(),
      e.what()
    );
    return;
  }

  std::ifstream snippetFile(path);
  if (!snippetFile.is_open())
  {
    mLogger->warn("Could not load '{}': failed to open", path.string());
    return;
  }

  std::stringstream buffer;
  buffer << snippetFile.rdbuf();
  const std::string &sbuffer = buffer.str();

  MQTT::Message message;

  if (!sbuffer.empty())
  {
    if (!nlohmann::json::accept(sbuffer))
    {
      mLogger->warn("Could not load '{}': malformed json", path.string());
      return;
    }

    auto j = nlohmann::json::parse(sbuffer);
    message = MQTT::Message::fromJson(j);
  }

  Node newNode {
    parentId,
    decoded,
    path.stem().string(),
    Node::Type::Snippet,
    {},
    message,
    true,
  };
  const auto newId = getNextId();
  mNodes.insert({newId, std::move(newNode)});
  mNodes.at(parentId).children.push_back(newId);
}

bool Snippets::indexFileRead(
  const std::filesystem::path &path,
  Node::Id_t id
) {
  const std::string indexPath = fmt::format(
    "{}/.index.json",
    path.string()
  );

  const auto &name = mNodes.at(id).name;

  if (!fs::exists(indexPath))
  {
    mLogger->error("Could not sort '{}': No .index.json found", name);
    return false;
  }

  std::ifstream indexFile(indexPath, std::ios::in);
  if (!indexFile.is_open())
  {
    mLogger->error("Could not sort '{}': Failed to open .index.json", name);
    return false;
  }

  nlohmann::json indexes;
  indexFile >> indexes;

  if (indexes.type() != nlohmann::json::value_t::array)
  {
    mLogger->error("Could not sort '{}': .index.json is not an array", name);
    fs::remove(indexPath);
    return false;
  }

  mNodes.at(id).children.sort(
    [this, &indexes](const Node::Id_t &lhs, const Node::Id_t &rhs) -> bool
    {
      const auto &lhsEnc = mNodes.at(lhs).encoded;
      const auto &rhsEnc = mNodes.at(rhs).encoded;

      auto lhsIt = std::find(
        std::begin(indexes),
        std::end(indexes),
        lhsEnc
      );
      auto rhsIt = std::find(
        std::begin(indexes),
        std::end(indexes),
        rhsEnc
      );

      return lhsIt < rhsIt;
    }
  );

  return true;
}

bool Snippets::indexFileWrite(Node::Id_t id)
{
  const auto nodePath = getNodePath(id);
  const auto indexPath = nodePath + "/.index.json";
  std::ofstream output(indexPath);
  if (!output.is_open())
  {
    mLogger->warn("Could not save '{}'", indexPath);
    return false;
  }

  auto &node = mNodes.at(id);

  nlohmann::json data;
  for (const auto &child : node.children)
  {
    data.push_back(mNodes.at(child).encoded);
  }
  output << data;

  node.saved = true;

  return true;
}

bool Snippets::hasChildNamed(
  wxDataViewItem parent,
  const std::string &name
) const {
  const auto id = toId(parent);
  const auto children = mNodes.at(id).children;
  return std::any_of(
    std::begin(children),
    std::end(children),
    [this, &name](const auto &child)
    {
      return mNodes.at(child).name == name;
    }
  );
}

unsigned Snippets::GetColumnCount() const
{
  return static_cast<unsigned>(Column::Max);
}

wxString Snippets::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Name: {
      return wxDataViewIconTextRenderer::GetDefaultType();
    } break;
    default: { return "string"; }
  }
}

void Snippets::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int /* col */
) const {
  auto id = toId(item);
  const auto &node = mNodes.at(id);
  wxDataViewIconText value;
  const auto wxs = wxString::FromUTF8(node.name.data(), node.name.length());
  value.SetText(wxs);
  switch (node.type)
  {
    case Node::Type::Folder: {
      value.SetIcon(wxArtProvider::GetIcon(wxART_FOLDER));
    } break;
    case Node::Type::Snippet: {
      value.SetIcon(wxArtProvider::GetIcon(wxART_NORMAL_FILE));
    } break;
  }
  variant << value;
}

bool Snippets::SetValue(
  const wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) {
  if (!item.IsOk())
  {
    return false;
  }
  if (static_cast<Column>(col) != Column::Name)
  {
    return false;
  }
  wxDataViewIconText iconText;
  iconText << value;

  const auto wxs = iconText.GetText();
  const auto utf8 = wxs.ToUTF8();
  const std::string newName(utf8.data(), utf8.length());

  if (newName == ".index")
  {
    mLogger->warn("Could not rename '{}' to '.index': name is reserved");
    return false;
  }

  auto parentItem = GetParent(item);
  if (hasChildNamed(parentItem, newName))
  {
    return false;
  }

  auto id = toId(item);
  auto &node = mNodes.at(id);
  if (node.name == newName) { return true; }
  if (newName.empty()) { return false; }

  const std::string encoded = Url::encode(newName);

  const auto parentPath = getNodePath(node.parent);
  const std::string pathOld = fmt::format(
    "{}/{}{}",
    parentPath,
    node.encoded,
    (node.type == Node::Type::Snippet) ? ".json" : ""
  );
  const std::string pathNew = fmt::format(
    "{}/{}{}",
    parentPath,
    encoded,
    (node.type == Node::Type::Snippet) ? ".json" : ""
  );

  std::error_code ec;
  fs::rename(pathOld, pathNew, ec);
  if (ec)
  {
    mLogger->error(
      "Could not rename '{}' to '{}': {}",
      pathOld,
      pathNew,
      ec.message()
    );
    return false;
  }

  node.name = newName;
  node.encoded = encoded;
  node.saved = false;
  mNodes.at(node.parent).saved = false;

  save(id);
  ItemChanged(item);

  return true;
}

bool Snippets::IsEnabled(
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) const {
  return true;
}

wxDataViewItem Snippets::GetParent(
  const wxDataViewItem &item
) const {

  if (!item.IsOk())
  {
    return wxDataViewItem(nullptr);
  }

  auto id = toId(item);
  auto parent = mNodes.at(id).parent;
  return toItem(parent);
}

bool Snippets::IsContainer(
  const wxDataViewItem &item
) const {

  if (!item.IsOk())
  {
    return true;
  }

  const auto &node = mNodes.at(toId(item));
  return node.type == Node::Type::Folder;
}

unsigned Snippets::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  Node::Id_t id = 0;

  if (parent.IsOk())
  {
    id = toId(parent);
  }

  const auto &node = mNodes.at(id);

  for (auto i : node.children)
  {
    array.Add(toItem(i));
  }
  return (unsigned)node.children.size();
}

Snippets::Node::Id_t Snippets::toId(const wxDataViewItem &item)
{
  uintptr_t result = 0;
  const void *id = item.GetID();
  std::memcpy(&result, &id, sizeof(item.GetID()));
  return result;
}

wxDataViewItem Snippets::toItem(Node::Id_t id)
{
  void *itemId = nullptr;
  const uintptr_t value = id;
  std::memcpy(&itemId, &value, sizeof(id));
  return wxDataViewItem(itemId);
}

Snippets::Node::Id_t Snippets::getNextId()
{
  return mNextAvailableId++;
}

bool Snippets::isRecursive(wxDataViewItem parent, wxDataViewItem item) const
{
  if (!parent.IsOk()) { return false; }
  if (parent == item) { return true; }
  return isRecursive(GetParent(parent), item);
}

std::string Snippets::getNodePath(Node::Id_t id) const
{
  Node::Id_t currentId = id;
  std::vector<std::string> parts;
  while (currentId != 0)
  {
    const auto &node = mNodes.at(currentId);
    parts.push_back(node.encoded);
    currentId = mNodes.at(currentId).parent;
  }

  parts.push_back(mNodes.at(0).encoded);

  std::string result;
  for (auto it = parts.rbegin(); it != parts.rend(); ++it)
  {
    result += *it + "/";
  }

  result.pop_back();

  if (mNodes.at(id).type == Node::Type::Snippet)
  {
    result += ".json";
  }

  return result;
}

bool Snippets::save(Node::Id_t id)
{
  auto &node = mNodes.at(id);
  if (node.saved) { return true; }
  if (id != 0 && !save(node.parent)) { return false; }

  switch (node.type)
  {
    case Node::Type::Snippet: return saveSnippet(id);
    case Node::Type::Folder:  return saveFolder(id);
  }
  return false;
}

bool Snippets::saveSnippet(Node::Id_t id)
{
  auto &node = mNodes.at(id);
  const auto nodePath = getNodePath(id);

  std::ofstream output(nodePath);
  if (!output.is_open())
  {
    mLogger->error("Could not save '{}'", node.name);
    return false;
  }

  output << MQTT::Message::toJson(node.message);

  node.saved = true;

  return true;
}

bool Snippets::saveFolder(Node::Id_t id)
{
  const auto nodePath = getNodePath(id);

  const bool exists = fs::exists(nodePath);
  const bool isDir = fs::is_directory(nodePath);

  if (exists && !isDir && !fs::remove(nodePath))
  {
    mLogger->warn("Could not remove file {}", nodePath);
    return false;
  }

  if (!exists && !fs::create_directory(nodePath))
  {
    mLogger->warn(
      "Could not create directory: {}",
      nodePath
    );
    return false;
  }

  indexFileWrite(id);

  return true;
}

bool Snippets::moveFile(Node::Id_t nodeId, Node::Id_t newParentId)
{
  const auto &node = mNodes.at(nodeId);

  if (node.parent == newParentId)
  {
    return true;
  }

  const std::string pathNew = fmt::format(
    "{}/{}{}",
    getNodePath(newParentId),
    node.encoded,
    (node.type == Node::Type::Snippet) ? ".json" : ""
  );

  if (fs::exists(pathNew))
  {
    mLogger->error("Could not move item: target exists");
    return false;
  }

  const auto pathOld = getNodePath(nodeId);
  std::error_code ec;
  fs::rename(pathOld, pathNew, ec);
  if (ec)
  {
    mLogger->error(
      "Could not move item: failed to rename '{}' to '{}': {}",
      pathOld,
      pathNew,
      ec.message()
    );
    return false;
  }

  return true;
}

bool Snippets::moveCheck(
  wxDataViewItem item,
  wxDataViewItem parent,
  size_t index
) {
  if (!item.IsOk())
  {
    mLogger->info("Could not move item: Item is null");
    return false;
  }

  if (item == parent)
  {
    mLogger->info("Could not move item: Item is Target");
    return false;
  }

  if (GetParent(item) == parent)
  {
    const auto parent = GetParent(item);
    wxDataViewItemArray children;
    GetChildren(parent, children);

    size_t currentIndex = 0;
    for (size_t i = 0; i != children.size(); ++i)
    {
      if (children[i].GetID() == item.GetID())
      {
        currentIndex = i;
        break;
      }
    }

    if (currentIndex == index)
    {
      mLogger->info("Did not move item: no action required");
      return false;
    }
  }

  if (isRecursive(parent, item))
  {
    mLogger->info("Could not move item: Target is recursive");
    return false;
  }

  return true;
}

void Snippets::moveUnderNewParent(
  Node::Id_t nodeId,
  Node::Id_t newParentId,
  size_t index
) {
  auto &node = mNodes.at(nodeId);
  const auto oldParentId = node.parent;
  auto &newParentNode = mNodes.at(newParentId);
  auto &oldParentNode = mNodes.at(oldParentId);

  auto &children = oldParentNode.children;
  auto removeIt = std::remove(
    std::begin(children),
    std::end(children),
    nodeId
  );
  children.erase(removeIt);

  mNodes.at(oldParentId).saved = false;
  save(oldParentId);

  node.parent = newParentId;

  if (index == 0)
  {
    newParentNode.children.push_front(nodeId);
  }
  else
  {
    auto &children = newParentNode.children;
    auto it = std::begin(children);
    std::advance(it, index);
    children.insert(it, nodeId);
  }
}

void Snippets::moveUnderSameParent(
  Node::Id_t nodeId,
  Node::Id_t newParentId,
  size_t index
) {
  auto &newParentNode = mNodes.at(newParentId);
  auto &siblings = newParentNode.children;

  std::list<Node::Id_t> temp;

  const auto oldIt = std::find(
    std::begin(siblings),
    std::end(siblings),
    nodeId
  );
  auto newIt = std::begin(siblings);
  std::advance(newIt, index);

  temp.splice(std::end(temp), siblings, oldIt);
  siblings.splice(newIt, temp);
}
