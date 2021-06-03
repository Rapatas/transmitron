#include "Snippets.hpp"

#include <algorithm>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string_view>
#include <wx/log.h>
#include <wx/artprov.h>
#include <fmt/format.h>
#include <cppcodec/base32_rfc4648.hpp>

#define wxLOG_COMPONENT "Models/Snippets" // NOLINT

namespace fs = std::filesystem;
using namespace Transmitron::Models;

Snippets::Snippets()
{
  const auto id = getNextId();
  Node root {
    0,
    "_",
    {},
    Node::Type::Folder,
    {},
    nullptr,
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

  if (!node.message)
  {
    return {};
  }

  return *node.message;
}

wxDataViewItem Snippets::getRootItem()
{
  return toItem(0);
}

bool Snippets::load(const std::string &connectionDir)
{
  if (connectionDir.empty())
  {
    wxLogWarning("No directory provided");
    return false;
  }

  mSnippetsDir = connectionDir + "/snippets";

  bool exists = fs::exists(mSnippetsDir);
  bool isDir = fs::is_directory(mSnippetsDir);

  if (exists && !isDir && !fs::remove(mSnippetsDir))
  {
    wxLogWarning("Could not remove file %s", mSnippetsDir);
    return false;
  }

  if (!exists && !fs::create_directory(mSnippetsDir))
  {
    wxLogWarning(
      "Could not create snippets directory: %s",
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

  wxLogInfo(
    "Creating folder '%s' under [%zu]'%s'",
    uniqueName,
    parentId,
    parentNode.name
  );

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(uniqueName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", uniqueName, e.what());
    return wxDataViewItem(nullptr);
  }

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
    nullptr,
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
  std::shared_ptr<MQTT::Message> message
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

  wxLogInfo(
    "Creating snippet '%s' under [%zu]'%s'",
    uniqueName,
    parentId,
    parentNode.name
  );

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(uniqueName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", uniqueName, e.what());
    return wxDataViewItem(nullptr);
  }

  const auto parentPath = getNodePath(parentId);
  const std::string path = fmt::format(
    "{}/{}",
    parentPath,
    encoded
  );

  Node newNode {
    parentId,
    uniqueName,
    encoded,
    Node::Type::Snippet,
    {},
    std::move(message),
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
  std::shared_ptr<MQTT::Message> message,
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

  wxLogInfo(
    "Creating snippet '%s' under [%zu]'%s'",
    name,
    parentId,
    parentNode.name
  );

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(name); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", name, e.what());
    return wxDataViewItem(nullptr);
  }

  const auto parentPath = getNodePath(parentId);
  const std::string path = fmt::format(
    "{}/{}",
    parentPath.c_str(),
    encoded
  );

  Node newNode {
    parentId,
    name,
    encoded,
    Node::Type::Snippet,
    {},
    std::move(message),
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
  std::shared_ptr<MQTT::Message> message
) {
  const auto id = toId(item);
  auto &node = mNodes.at(id);
  if (node.type != Node::Type::Snippet)
  {
    wxLogWarning("Can only replace snippets");
    return wxDataViewItem(nullptr);
  }

  const auto path = getNodePath(id);
  std::ofstream output(path);
  if (!output.is_open())
  {
    wxLogWarning("Could not open '%s'", path.c_str());
    return wxDataViewItem(nullptr);
  }

  if (message)
  {
    output << MQTT::Message::toJson(*message);
  }
  node.message = std::move(message);
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
    wxLogError("Could not delete '%s': %s", node.name, ec.message());
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
  return move(item, GetParent(sibling), sibling);
}

wxDataViewItem Snippets::moveInside(
  wxDataViewItem item,
  wxDataViewItem parent
) {
  return move(item, parent, wxDataViewItem(0));
}

wxDataViewItem Snippets::move(
  wxDataViewItem item,
  wxDataViewItem parent,
  wxDataViewItem sibling
) {
  if (!item.IsOk())
  {
    wxLogInfo("Could not move item: Item is null");
    return wxDataViewItem(0);
  }

  if (!parent.IsOk() && !sibling.IsOk())
  {
    wxLogInfo("Could not move item: Target it null");
    return wxDataViewItem(0);
  }

  if (item == parent)
  {
    wxLogInfo("Could not move item: Item is Target");
    return wxDataViewItem(0);
  }

  if (isRecursive(parent, item))
  {
    wxLogInfo("Could not move item: Target is recursive");
    return wxDataViewItem(0);
  }

  const auto nodeId = toId(item);
  auto &node = mNodes.at(nodeId);
  const auto newParentId = toId(parent);
  const auto oldParentId = node.parent;
  auto &newParentNode = mNodes.at(newParentId);
  auto &oldParentNode = mNodes.at(oldParentId);

  wxLogInfo(
    "Moving %zu in %zu before %zu",
    nodeId,
    newParentId,
    toId(sibling)
  );

  if (parent != GetParent(item))
  {
    const std::string pathNew = fmt::format(
      "{}/{}",
      getNodePath(newParentId),
      node.encoded
    );

    if (fs::exists(pathNew))
    {
      wxLogError("Could not move item: target exists");
      return wxDataViewItem(0);
    }

    const auto pathOld = getNodePath(nodeId);
    std::error_code ec;
    fs::rename(pathOld, pathNew, ec);
    if (ec)
    {
      wxLogError(
        "Could not move item: failed to rename '%s' to '%s': %s",
        pathOld,
        pathNew,
        ec.message()
      );
      return wxDataViewItem(0);
    }

    auto &children = oldParentNode.children;
    auto removeIt = std::remove(
      std::begin(children),
      std::end(children),
      nodeId
    );
    children.erase(removeIt);

    node.parent = newParentId;

    mNodes.at(oldParentId).saved = false;
    save(oldParentId);
  }

  ItemDeleted(toItem(oldParentId), item);
  mNodes.at(newParentId).saved = false;

  if (!sibling.IsOk())
  {
    newParentNode.children.push_front(nodeId);
  }
  else if (oldParentId != newParentId)
  {
    auto &children = newParentNode.children;
    auto it = std::find(
      std::begin(children),
      std::end(children),
      toId(sibling)
    );
    children.insert(it, nodeId);
  }
  else
  {
    auto &siblings = newParentNode.children;

    std::list<Node::Id_t> temp;

    const auto oldIt = std::find(
      std::begin(siblings),
      std::end(siblings),
      nodeId
    );
    const auto newIt = std::find(
      std::begin(siblings),
      std::end(siblings),
      toId(sibling)
    );

    temp.splice(std::end(temp), siblings, oldIt);

    siblings.splice(newIt, temp);
  }

  ItemAdded(parent, item);
  for (const auto &child : node.children)
  {
    ItemAdded(item, toItem(child));
  }
  save(newParentId);

  return item;
}

void Snippets::loadDirectoryRecursive(
  const std::filesystem::path &path,
  Node::Id_t parentId
) {
  const bool isRoot = parentId == std::numeric_limits<Node::Id_t>::max();

  const auto currentId = isRoot ? 0   : getNextId();
  const auto name      = isRoot ? "_" : decode(path.stem());

  if (!isRoot)
  {
    Node newNode {
      parentId,
      name,
      path.filename(),
      Node::Type::Folder,
      {},
      nullptr,
      true,
    };

    mNodes.insert({currentId, std::move(newNode)});
    mNodes.at(parentId).children.push_back(currentId);
  }

  for (const auto &entry : fs::directory_iterator(path))
  {
    if (entry.path().stem() == "index") { continue; }

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

  const auto indexPath = path.native() + "/index.json";
  if (!fs::exists(indexPath))
  {
    wxLogError("Could not sort '%s': No index.json found", name);
    return;
  }

  std::ifstream indexFile(indexPath);
  if (!indexFile.is_open())
  {
    wxLogError("Could not sort '%s': Failed to open index.json", name);
    return;
  }

  nlohmann::json indexes;
  indexFile >> indexes;

  if (indexes.type() != nlohmann::json::value_t::array)
  {
    wxLogError("Could not sort '%s': index.json is not an array", name);
    fs::remove(indexPath);
    return;
  }

  mNodes.at(currentId).children.sort(
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
}

void Snippets::loadSnippet(
  const std::filesystem::path &path,
  Node::Id_t parentId
) {
  const auto stem = path.stem().u8string();
  const std::string name = decode(stem);

  std::ifstream snippetFile(path);
  if (!snippetFile.is_open())
  {
    wxLogWarning("Could not load '%s': failed to open", path.c_str());
    return;
  }

  std::stringstream buffer;
  buffer << snippetFile.rdbuf();
  const std::string &sbuffer = buffer.str();

  std::unique_ptr<MQTT::Message> message;

  if (!sbuffer.empty())
  {
    if (!nlohmann::json::accept(sbuffer))
    {
      wxLogWarning("Could not load '%s': malformed json", path.c_str());
      return;
    }

    auto j = nlohmann::json::parse(sbuffer);
    message = std::make_unique<MQTT::Message>(MQTT::Message::fromJson(j));
  }

  Node newNode {
    parentId,
    name,
    path.filename(),
    Node::Type::Snippet,
    {},
    std::move(message),
    true,
  };
  const auto newId = getNextId();
  mNodes.insert({newId, std::move(newNode)});
  mNodes.at(parentId).children.push_back(newId);
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
  value.SetText(node.name);
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
  const std::string newName = iconText.GetText().ToStdString();

  auto parentItem = GetParent(item);
  if (hasChildNamed(parentItem, newName))
  {
    return false;
  }

  auto id = toId(item);
  auto &node = mNodes.at(id);
  if (node.name == newName) { return true; }
  if (newName.empty()) { return false; }

  wxLogInfo("Renaming '%zu' to '%s'", id, newName);

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(newName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", newName, e.what());
    return false;
  }

  const auto parentPath = getNodePath(node.parent);
  const std::string pathOld = fmt::format(
    "{}/{}",
    parentPath,
    node.encoded
  );
  const std::string pathNew = fmt::format(
    "{}/{}",
    parentPath,
    encoded
  );

  std::error_code ec;
  fs::rename(pathOld, pathNew, ec);
  if (ec)
  {
    wxLogError(
      "Could not rename '%s' to '%s': %s",
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

  return result;
}

bool Snippets::save(Node::Id_t id)
{
  auto &node = mNodes.at(id);
  if (node.saved) { return true; }
  if (id != 0 && !save(node.parent)) { return false; }

  const auto nodePath = getNodePath(id);

  if (node.type == Node::Type::Snippet)
  {
    std::ofstream output(nodePath);
    if (!output.is_open())
    {
      wxLogError("Could not save '%s'", node.name);
      return false;
    }

    if (node.message)
    {
      output << MQTT::Message::toJson(*node.message);
    }
  }
  else if (node.type == Node::Type::Folder)
  {
    bool exists = fs::exists(nodePath);
    bool isDir = fs::is_directory(nodePath);

    if (exists && !isDir && !fs::remove(nodePath))
    {
      wxLogWarning("Could not remove file %s", nodePath.c_str());
      return false;
    }

    if (!exists && !fs::create_directory(nodePath))
    {
      wxLogWarning(
        "Could not create directory: %s",
        nodePath.c_str()
      );
      return false;
    }

    const auto indexPath = nodePath + "/index.json";
    std::ofstream output(indexPath);
    if (!output.is_open())
    {
      wxLogWarning("Could not save '%s'", indexPath);
    }
    else
    {
      nlohmann::json data;
      for (const auto &child : node.children)
      {
        data.push_back(mNodes.at(child).encoded);
      }
      output << data;
    }
  }

  node.saved = true;

  return true;
}

std::string Snippets::decode(const std::string &encoded)
{
  std::vector<uint8_t> decoded;

  try
  {
    decoded = cppcodec::base32_rfc4648::decode(encoded);
  }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not decode '%s': %s", encoded, e.what());
    return encoded;
  }

  return {decoded.begin(), decoded.end()};
}
