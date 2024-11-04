#include "FsTree.hpp"

#include <algorithm>
#include <fstream>
#include <ios>
#include <iterator>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <string_view>

#include "Common/Log.hpp"
#include "Common/Url.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Models;
using namespace Common;

using Leaf = FsTree::Leaf;

FsTree::FsTree(
  const std::string &name,
  size_t columnCount,
  const ArtProvider &artProvider
) :
  mArtProvider(artProvider),
  mColumnCount(columnCount) //
{
  mLogger = Common::Log::create(fmt::format("Models::{}", name));
  clear();
}

void FsTree::clear() {
  mNodes.clear();
  mNextAvailableId = 0;

  const auto id = getNextId();
  Node root{0, "_", {}, Node::Type::Folder, {}, nullptr, true};

  mNodes.insert({id, std::move(root)});
}

Leaf *FsTree::getLeaf(wxDataViewItem item) const {
  if (!item.IsOk()) { return nullptr; }

  const auto id = toId(item);
  const auto nodeIt = mNodes.find(id);
  if (nodeIt == mNodes.end()) { return nullptr; }
  return nodeIt->second.payload.get();
}

wxDataViewItem FsTree::getItemFromName(const std::string &name) const {
  const auto it = std::find_if(
    std::begin(mNodes),
    std::end(mNodes),
    [&](const auto &element) {
      const auto &node = element.second;
      return node.name == name;
    }
  );

  if (it == std::end(mNodes)) { return wxDataViewItem(nullptr); }

  return toItem(it->first);
}

std::string FsTree::getName(wxDataViewItem item) const {
  if (!item.IsOk()) { return {}; }

  const auto id = toId(item);
  const auto &node = mNodes.at(id);
  return node.name;
}

std::map<FsTree::Id, Leaf *> FsTree::getLeafs() const {
  std::map<Id, Leaf *> result;
  for (const auto &[nodeId, node] : mNodes) {
    if (node.type != Node::Type::Payload) { continue; }
    result[nodeId] = node.payload.get();
  }
  return result;
}

wxDataViewItem FsTree::getRootItem() { return toItem(0); }

bool FsTree::load(const std::string &baseDir) {
  clear();

  mLogger->debug("load(): base: {}", baseDir);
  if (baseDir.empty()) {
    mLogger->warn("No directory provided");
    return false;
  }

  mBaseDir = baseDir;

  const bool exists = fs::exists(mBaseDir);
  const bool isDir = fs::is_directory(mBaseDir);

  if (exists && !isDir && !fs::remove(mBaseDir)) {
    mLogger->warn("Could not remove file {}", mBaseDir);
    return false;
  }

  if (!exists && !fs::create_directory(mBaseDir)) {
    mLogger->warn("Could not create messages directory: {}", mBaseDir);
    return false;
  }

  mNodes.at(0).encoded = mBaseDir;

  loadDirectoryRecursive(mBaseDir, std::numeric_limits<Id>::max());

  return true;
}

wxDataViewItem FsTree::createFolder(wxDataViewItem parentItem) {
  const constexpr std::string_view NewName{"New Folder"};

  auto parentId = toId(parentItem);
  const auto &parentNode = mNodes.at(parentId);
  if (parentNode.type != Node::Type::Folder) { return wxDataViewItem(nullptr); }

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

  Node newNode{
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

std::string FsTree::createUniqueName(
  wxDataViewItem parent,
  std::string_view name
) const {
  // Find a unique name.
  std::string uniqueName{name};
  size_t postfix = 0;
  while (hasChildNamed(parent, uniqueName)) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", name, postfix);
  }
  return uniqueName;
}

wxDataViewItem FsTree::leafCreate(
  wxDataViewItem parentItem,
  std::unique_ptr<Leaf> leaf,
  const std::string &name
) {
  auto parentId = toId(parentItem);
  const auto &parentNode = mNodes.at(parentId);
  if (parentNode.type != Node::Type::Folder) { return wxDataViewItem(nullptr); }

  mLogger->info(
    "Creating leaf '{}' under [{}]'{}'",
    name,
    parentId,
    parentNode.name
  );

  const std::string encoded = Url::encode(name);

  Node newNode{
    parentId,
    name,
    encoded,
    Node::Type::Payload,
    {},
    std::move(leaf),
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

wxDataViewItem FsTree::leafInsert(
  const std::string &name,
  std::unique_ptr<Leaf> leaf,
  wxDataViewItem parentItem
) {
  const auto parentId = toId(parentItem);
  const auto &parentNode = mNodes.at(parentId);

  if (parentNode.type != Node::Type::Folder) { return wxDataViewItem(nullptr); }
  if (hasChildNamed(parentItem, name)) { return wxDataViewItem(nullptr); }

  mLogger->info(
    "Creating message '{}' under [{}]'{}'",
    name,
    parentId,
    parentNode.name
  );

  const std::string encoded = Url::encode(name);

  Node newNode{
    parentId,
    name,
    encoded,
    Node::Type::Payload,
    {},
    std::move(leaf),
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

wxDataViewItem FsTree::leafReplace(
  wxDataViewItem item,
  std::unique_ptr<Leaf> leaf
) {
  const auto id = toId(item);
  auto &node = mNodes.at(id);
  if (node.type != Node::Type::Payload) {
    mLogger->warn("Can only replace messages");
    return wxDataViewItem(nullptr);
  }

  node.payload = std::move(leaf);
  node.saved = false;
  save(id);

  return item;
}

bool FsTree::rename(wxDataViewItem item, const std::string &name) {
  auto id = toId(item);
  auto &node = mNodes.at(id);

  const std::string encoded = Url::encode(name);

  const auto pathOld = getNodePath(id);
  node.encoded = encoded;
  const auto pathNew = getNodePath(id);

  std::error_code ec;
  fs::rename(pathOld, pathNew, ec);
  if (ec) {
    mLogger->error(
      "Could not rename '{}' to '{}': {}",
      pathOld,
      pathNew,
      ec.message()
    );
    return false;
  }

  node.name = name;
  node.encoded = encoded;
  node.saved = false;
  mNodes.at(node.parent).saved = false;

  save(id);
  ItemChanged(item);

  return true;
}

bool FsTree::remove(wxDataViewItem item) {
  const auto id = toId(item);
  auto &node = mNodes.at(id);
  const auto parent = GetParent(item);

  std::error_code ec;
  const auto path = getNodePath(id);
  fs::remove_all(path, ec);
  if (ec) {
    mLogger->error("Could not delete '{}': {}", node.name, ec.message());
    return false;
  }

  auto &children = mNodes.at(toId(parent)).children;
  auto removeIt = std::remove(std::begin(children), std::end(children), id);
  children.erase(removeIt);
  mNodes.erase(id);
  ItemDeleted(parent, item);

  return true;
}

wxDataViewItem FsTree::moveBefore(wxDataViewItem item, wxDataViewItem sibling) {
  const auto parent = GetParent(sibling);
  wxDataViewItemArray children;
  GetChildren(parent, children);

  size_t index = 0;
  for (size_t i = 0; i != children.size(); ++i) {
    if (children[i].GetID() == sibling.GetID()) {
      index = i;
      break;
    }
  }

  return moveInsideAtIndex(item, parent, index);
}

wxDataViewItem FsTree::moveInsideLast(
  wxDataViewItem item,
  wxDataViewItem parent
) {
  wxDataViewItemArray children;
  const auto count = GetChildren(parent, children);
  return moveInsideAtIndex(item, parent, count);
}

wxDataViewItem FsTree::moveAfter(wxDataViewItem item, wxDataViewItem sibling) {
  const auto parent = GetParent(sibling);
  wxDataViewItemArray children;
  GetChildren(parent, children);

  size_t index = 0;
  for (size_t i = 0; i != children.size(); ++i) {
    if (children[i].GetID() == sibling.GetID()) {
      index = i + 1;
      break;
    }
  }

  return moveInsideAtIndex(item, parent, index);
}

wxDataViewItem FsTree::moveInsideFirst(
  wxDataViewItem item,
  wxDataViewItem parent
) {
  return moveInsideAtIndex(item, parent, 0);
}

wxDataViewItem FsTree::moveInsideAtIndex(
  wxDataViewItem item,
  wxDataViewItem parent,
  size_t index
) {
  if (!moveCheck(item, parent, index)) { return wxDataViewItem(nullptr); }

  const auto nodeId = toId(item);
  const auto &node = mNodes.at(nodeId);
  const auto newParentId = toId(parent);
  const auto oldParentId = node.parent;

  if (!moveFile(nodeId, newParentId)) { return wxDataViewItem(nullptr); }

  if (oldParentId != newParentId) {
    mLogger->info("Moving {} in {} at index {}", nodeId, newParentId, index);
    moveUnderNewParent(nodeId, newParentId, index);
  } else {
    mLogger->info("Moving {} at index {}", nodeId, newParentId, index);
    moveUnderSameParent(nodeId, newParentId, index);
  }

  ItemDeleted(toItem(oldParentId), item);
  ItemAdded(parent, item);
  for (const auto &child : node.children) { ItemAdded(item, toItem(child)); }

  mNodes.at(oldParentId).saved = false;
  mNodes.at(nodeId).saved = false;
  mNodes.at(newParentId).saved = false;
  save(newParentId);
  save(oldParentId);

  return item;
}

void FsTree::loadDirectoryRecursive(const Common::fs::path &path, Id parentId) {
  const bool isRoot = parentId == std::numeric_limits<Id>::max();
  const auto currentId = isRoot ? 0 : getNextId();
  std::string decoded = "_";

  // mLogger->debug("loadDirectoryRecursive: {}", path.string());

  if (!isRoot) {
    try {
      decoded = Url::decode(path.filename().string());
    } catch (std::runtime_error &error) {
      mLogger->error("Could not decode '{}': {}", path.string(), error.what());
      return;
    }

    Node newNode{
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

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.path().stem() == ".index") { continue; }

    if (isLeaf(entry)) {
      mLogger->trace("  - detected! {}", entry.path().string());
      loadLeaf(entry, currentId);
    } else if (entry.status().type() == fs::file_type::directory) {
      // mLogger->trace("  - is directory! {}", entry.path().string());
      loadDirectoryRecursive(entry.path(), currentId);
    }
  }

  if (mNodes.at(currentId).children.empty()) { return; }

  const bool read = indexFileRead(path, currentId);
  if (!read) { indexFileWrite(currentId); }
}

void FsTree::loadLeaf(const Common::fs::directory_entry &entry, Id parentId) {
  std::string decoded;
  const auto &path = entry.path();
  try {
    const auto real = (entry.status().type() == fs::file_type::directory)
      ? path.filename()
      : path.stem();

    decoded = Url::decode(real.string());
  } catch (std::runtime_error &error) {
    mLogger->error("Could not decode '{}': {}", path.string(), error.what());
    return;
  }

  Node newNode{
    parentId,
    decoded,
    path.stem().string(),
    Node::Type::Payload,
    {},
    nullptr,
    true,
  };
  const auto newId = getNextId();
  auto &node = mNodes.insert({newId, std::move(newNode)}).first->second;
  mNodes.at(parentId).children.push_back(newId);

  auto leaf = leafLoad(newId, path);
  node.payload = std::move(leaf);
}

bool FsTree::indexFileRead(const Common::fs::path &path, Id id) {
  const std::string indexPath = fmt::format("{}/.index.json", path.string());

  const auto &name = mNodes.at(id).name;

  if (!fs::exists(indexPath)) {
    mLogger->error("Could not sort '{}': No .index.json found", name);
    return false;
  }

  std::ifstream indexFile(indexPath, std::ios::in);
  if (!indexFile.is_open()) {
    mLogger->error("Could not sort '{}': Failed to open .index.json", name);
    return false;
  }

  nlohmann::json indexes;
  indexFile >> indexes;

  if (indexes.type() != nlohmann::json::value_t::array) {
    mLogger->error("Could not sort '{}': .index.json is not an array", name);
    fs::remove(indexPath);
    return false;
  }

  mNodes.at(id).children.sort(
    [this, &indexes](const Id &lhs, const Id &rhs) -> bool {
      const auto &lhsEnc = mNodes.at(lhs).encoded;
      const auto &rhsEnc = mNodes.at(rhs).encoded;

      auto lhsIt = std::find(std::begin(indexes), std::end(indexes), lhsEnc);
      auto rhsIt = std::find(std::begin(indexes), std::end(indexes), rhsEnc);

      return lhsIt < rhsIt;
    }
  );

  return true;
}

bool FsTree::indexFileWrite(Id id) {
  const auto nodePath = getNodePath(id);
  const auto indexPath = nodePath + "/.index.json";
  std::ofstream output(indexPath);
  if (!output.is_open()) {
    mLogger->warn("Could not save '{}'", indexPath);
    return false;
  }

  auto &node = mNodes.at(id);

  nlohmann::json data;
  for (const auto &child : node.children) {
    data.push_back(mNodes.at(child).encoded);
  }
  output << data;

  node.saved = true;

  return true;
}

bool FsTree::hasChildNamed(wxDataViewItem parent, const std::string &name)
  const {
  const auto id = toId(parent);
  const auto children = mNodes.at(id).children;
  return std::any_of(
    std::begin(children),
    std::end(children),
    [this, &name](const auto &child) { return mNodes.at(child).name == name; }
  );
}

unsigned FsTree::GetColumnCount() const {
  return static_cast<unsigned>(mColumnCount);
}

wxString FsTree::GetColumnType(unsigned int col) const {
  (void)col;
  return wxDataViewIconTextRenderer::GetDefaultType();
}

void FsTree::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  auto id = toId(item);
  const auto &node = mNodes.at(id);
  wxDataViewIconText value;
  const auto wxs = wxString::FromUTF8(node.name.data(), node.name.length());
  value.SetText(wxs);
  switch (node.type) {
    case Node::Type::Folder: {
      wxIcon icon;
      icon.CopyFromBitmap(mArtProvider.bitmap(Icon::Folder));
      value.SetIcon(icon);
    } break;
    case Node::Type::Payload: {
      leafValue(id, value, col);
    } break;
  }
  variant << value;
}

bool FsTree::SetValue(
  const wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) {
  if (!item.IsOk()) { return false; }
  if (static_cast<Column>(col) != Column::Name) { return false; }
  wxDataViewIconText iconText;
  iconText << value;

  const auto wxs = iconText.GetText();
  const auto utf8 = wxs.ToUTF8();
  const std::string newName(utf8.data(), utf8.length());

  if (newName == ".index") {
    mLogger->warn("Could not rename '{}' to '.index': name is reserved");
    return false;
  }

  auto parentItem = GetParent(item);
  if (hasChildNamed(parentItem, newName)) { return false; }

  auto id = toId(item);
  auto &node = mNodes.at(id);
  if (node.name == newName) { return true; }
  if (newName.empty()) { return false; }

  return rename(item, newName);
}

bool FsTree::IsEnabled(
  const wxDataViewItem & /* item */,
  unsigned int /* col */
) const {
  return true;
}

wxDataViewItem FsTree::GetParent(const wxDataViewItem &item) const {
  if (!item.IsOk()) { return wxDataViewItem(nullptr); }

  auto id = toId(item);
  auto parent = mNodes.at(id).parent;
  return toItem(parent);
}

bool FsTree::IsContainer(const wxDataViewItem &item) const {
  if (!item.IsOk()) { return true; }

  const auto &node = mNodes.at(toId(item));
  return node.type == Node::Type::Folder;
}

unsigned FsTree::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  Id id = 0;

  if (parent.IsOk()) { id = toId(parent); }

  const auto &node = mNodes.at(id);

  for (auto childId : node.children) { array.Add(toItem(childId)); }
  return static_cast<unsigned>(node.children.size());
}

FsTree::Id FsTree::toId(const wxDataViewItem &item) {
  uintptr_t result = 0;
  const void *id = item.GetID();
  std::memcpy(&result, &id, sizeof(item.GetID()));
  return result;
}

wxDataViewItem FsTree::toItem(Id id) {
  void *itemId = nullptr;
  const uintptr_t value = id;
  std::memcpy(&itemId, &value, sizeof(id));
  return wxDataViewItem(itemId);
}

FsTree::Id FsTree::getNextId() { return mNextAvailableId++; }

bool FsTree::isRecursive(wxDataViewItem parent, wxDataViewItem item) const {
  if (!parent.IsOk()) { return false; }
  if (parent == item) { return true; }
  return isRecursive(GetParent(parent), item);
}

std::string FsTree::getNodePath(Id id) const {
  auto parts = getNodeSegments(id);
  parts.push_back(mNodes.at(0).encoded);

  std::string result;
  for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
    result += *it + "/";
  }

  result.pop_back();

  return result;
}

std::vector<std::string> FsTree::getNodeSegments(Id id) const {
  Id currentId = id;
  std::vector<std::string> parts;
  while (currentId != 0) {
    const auto &node = mNodes.at(currentId);
    parts.push_back(node.encoded);
    currentId = mNodes.at(currentId).parent;
  }
  return parts;
}

bool FsTree::save(Id id) {
  auto &node = mNodes.at(id);
  if (node.saved) { return true; }
  if (id != 0 && !save(node.parent)) { return false; }

  switch (node.type) {
    case Node::Type::Payload: return saveLeaf(id);
    case Node::Type::Folder: return saveFolder(id);
  }
  return false;
}

bool FsTree::saveLeaf(Id id) {
  mLogger->debug("Saving leaf {}", id);
  auto &node = mNodes.at(id);
  if (!leafSave(id)) {
    mLogger->error("Could not save '{}'", node.name);
    return false;
  }

  node.saved = true;
  return true;
}

bool FsTree::saveFolder(Id id) {
  mLogger->debug("Saving folder {}", id);
  const auto nodePath = getNodePath(id);

  const bool exists = fs::exists(nodePath);
  const bool isDir = fs::is_directory(nodePath);

  if (exists && !isDir && !fs::remove(nodePath)) {
    mLogger->warn("Could not remove file {}", nodePath);
    return false;
  }

  if (!exists && !fs::create_directory(nodePath)) {
    mLogger->warn("Could not create directory: {}", nodePath);
    return false;
  }

  indexFileWrite(id);

  const auto &node = mNodes.at(id);
  for (const auto &childId : node.children) { save(childId); }

  return true;
}

bool FsTree::moveFile(Id nodeId, Id newParentId) {
  const auto &node = mNodes.at(nodeId);

  if (node.parent == newParentId) { return true; }

  const std::string pathNew = fmt::format(
    "{}/{}",
    getNodePath(newParentId),
    node.encoded
  );

  if (fs::exists(pathNew)) {
    mLogger->error("Could not move item: target exists");
    return false;
  }

  const auto pathOld = getNodePath(nodeId);
  std::error_code ec;
  fs::rename(pathOld, pathNew, ec);
  if (ec) {
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

bool FsTree::moveCheck(
  wxDataViewItem item,
  wxDataViewItem parent,
  size_t index
) {
  if (!item.IsOk()) {
    mLogger->info("Could not move item: Item is null");
    return false;
  }

  if (item == parent) {
    mLogger->info("Could not move item: Item is Target");
    return false;
  }

  if (GetParent(item) == parent) {
    const auto parent = GetParent(item);
    wxDataViewItemArray children;
    GetChildren(parent, children);

    size_t currentIndex = 0;
    for (size_t i = 0; i != children.size(); ++i) {
      if (children[i].GetID() == item.GetID()) {
        currentIndex = i;
        break;
      }
    }

    if (currentIndex == index) {
      mLogger->info("Did not move item: no action required");
      return false;
    }
  }

  if (isRecursive(parent, item)) {
    mLogger->info("Could not move item: Target is recursive");
    return false;
  }

  return true;
}

void FsTree::moveUnderNewParent(Id nodeId, Id newParentId, size_t index) {
  auto &node = mNodes.at(nodeId);
  const auto oldParentId = node.parent;
  auto &newParentNode = mNodes.at(newParentId);
  auto &oldParentNode = mNodes.at(oldParentId);

  auto &children = oldParentNode.children;
  auto removeIt = std::remove(std::begin(children), std::end(children), nodeId);
  children.erase(removeIt);

  mNodes.at(oldParentId).saved = false;
  save(oldParentId);

  node.parent = newParentId;

  if (index == 0) {
    newParentNode.children.push_front(nodeId);
  } else {
    auto &children = newParentNode.children;
    auto it = std::begin(children);
    std::advance(it, index);
    children.insert(it, nodeId);
  }
}

void FsTree::moveUnderSameParent(Id nodeId, Id newParentId, size_t index) {
  auto &newParentNode = mNodes.at(newParentId);
  auto &siblings = newParentNode.children;

  std::list<Id> temp;

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
