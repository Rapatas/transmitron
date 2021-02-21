#include "Snippets.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include <wx/log.h>
#include <fmt/format.h>
#include <cppcodec/base32_rfc4648.hpp>

#define wxLOG_COMPONENT "Models/Snippets"

namespace fs = std::filesystem;
using namespace Transmitron::Models;

Snippets::Snippets()
{
  const auto index = getNextIndex();
  Node root {
    0,
    "_",
    Node::Type::Folder,
    {},
    nullptr,
    true,
    {}
  };

  mNodes.insert({index, std::move(root)});
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

  wxLogDebug(
    "exists=%s, isDir=%s",
    (exists ? "yes" : "no"),
    (isDir ? "yes" : "no")
  );

  if (exists && !isDir && !fs::remove(mSnippetsDir))
  {
    wxLogWarning("Could not remove file %s", mSnippetsDir);
    return false;
  }

  if (!exists && !fs::create_directory(mSnippetsDir))
  {
    wxLogWarning(
      "Could not create directory: %s",
      mSnippetsDir
    );
    return false;
  }

  mNodes.at(0).fullpath = mSnippetsDir;

  loadRecursive(0, mSnippetsDir);

  return true;
}

wxDataViewItem Snippets::createFolder(
  wxDataViewItem parentItem
) {
  const constexpr char *newName = "New Folder";

  auto parentIndex = toIndex(parentItem);
  Node &parentNode = mNodes.at(parentIndex);
  if (parentNode.type != Node::Type::Folder)
  {
    return wxDataViewItem(nullptr);
  }

  // Find a unique name.
  std::string uniqueName = newName;
  size_t postfix = 0;
  while (hasChildNamed(parentItem, uniqueName)) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", newName, postfix);
  }

  wxLogInfo(
    "Creating folder '%s' under [%zu]'%s'",
    uniqueName,
    parentIndex,
    parentNode.name
  );

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(uniqueName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", uniqueName, e.what());
    return wxDataViewItem(nullptr);
  }

  const std::string fullpath = fmt::format(
    "{}/{}",
    parentNode.fullpath.c_str(),
    encoded
  );

  Node newNode {
    parentIndex,
    uniqueName,
    Node::Type::Folder,
    {},
    nullptr,
    false,
    fullpath
  };
  const auto newIndex = getNextIndex();
  mNodes.insert({newIndex, std::move(newNode)});
  mNodes[parentIndex].children.insert(newIndex);

  save(newIndex);

  const auto item = toItem(newIndex);
  ItemAdded(parentItem, item);

  return item;
}

wxDataViewItem Snippets::insert(
  const std::string &name,
  std::shared_ptr<MQTT::Message> message,
  wxDataViewItem parentItem
) {
  const auto parentIndex = toIndex(parentItem);
  const auto &parentNode = mNodes.at(parentIndex);

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
    parentIndex,
    parentNode.name
  );

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(name); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", name, e.what());
    return wxDataViewItem(nullptr);
  }

  const std::string fullpath = fmt::format(
    "{}/{}",
    parentNode.fullpath.c_str(),
    encoded
  );

  Node newNode {
    parentIndex,
    name,
    Node::Type::Snippet,
    {},
    message,
    false,
    fullpath
  };
  const auto newIndex = getNextIndex();
  mNodes.insert({newIndex, std::move(newNode)});
  mNodes[parentIndex].children.insert(newIndex);

  save(newIndex);

  const auto item = toItem(newIndex);
  ItemAdded(parentItem, item);

  return item;
}

wxDataViewItem Snippets::replace(
  wxDataViewItem item,
  std::shared_ptr<MQTT::Message> message
) {
  const auto index = toIndex(item);
  auto &node = mNodes.at(index);
  if (node.type != Node::Type::Snippet)
  {
    wxLogWarning("Can only replace snippets");
    return wxDataViewItem(nullptr);
  }

  std::ofstream output(node.fullpath);
  if (!output.is_open())
  {
    wxLogWarning("Could not open '%s'", node.fullpath.c_str());
    return wxDataViewItem(nullptr);
  }

  output << MQTT::Message::toJson(*message);
  node.message = message;
  return item;
}

bool Snippets::remove(wxDataViewItem item)
{
  const auto index = toIndex(item);
  auto &node = mNodes.at(index);
  const auto parent = GetParent(item);

  std::error_code ec;
  fs::remove_all(node.fullpath, ec);
  if (ec)
  {
    wxLogError("Could not delete '%s': %s", node.name, ec.message());
    return false;
  }

  mNodes.at(toIndex(parent)).children.erase(index);
  mNodes.erase(index);
  ItemDeleted(parent, item);

  return true;
}

void Snippets::loadRecursive(
  Node::Index_t parentIndex,
  const std::filesystem::path &parentFullpath
) {
  for (const auto &entry : fs::directory_iterator(parentFullpath))
  {
    std::vector<uint8_t> decoded;
    const auto stem = entry.path().stem().u8string();
    try {
      decoded = cppcodec::base32_rfc4648::decode(stem);
    }
    catch (cppcodec::parse_error &e)
    {
      wxLogError("Could not decode '%s': %s", stem, e.what());
      continue;
    }
    std::string name{decoded.begin(), decoded.end()};

    if (entry.status().type() == fs::file_type::directory)
    {
      Node newNode {
        parentIndex,
        name,
        Node::Type::Folder,
        {},
        nullptr,
        true,
        entry.path()
      };
      const auto newIndex = getNextIndex();
      mNodes.insert({newIndex, std::move(newNode)});
      mNodes[parentIndex].children.insert(newIndex);
      loadRecursive(newIndex, entry.path());
    }
    else
    {
      std::ifstream snippetFile(entry.path());
      if (!snippetFile.is_open())
      {
        wxLogWarning("Could not open '%s'", entry.path().c_str());
        continue;
      }

      std::stringstream buffer;
      buffer << snippetFile.rdbuf();
      const std::string &sbuffer = buffer.str();
      if (!nlohmann::json::accept(sbuffer))
      {
        wxLogWarning("Could not parse '%s'", entry.path().c_str());
        continue;
      }

      auto j = nlohmann::json::parse(sbuffer);
      auto message = std::make_unique<MQTT::Message>(MQTT::Message::fromJson(j));

      Node newNode {
        parentIndex,
        name,
        Node::Type::Snippet,
        {},
        std::move(message),
        true,
        entry.path()
      };
      const auto newIndex = getNextIndex();
      mNodes.insert({newIndex, std::move(newNode)});
      mNodes[parentIndex].children.insert(newIndex);
    }
  }
}

bool Snippets::hasChildNamed(
  wxDataViewItem parent,
  const std::string &name
) const {
  const auto index = toIndex(parent);
  const auto children = mNodes.at(index).children;
  for (const auto &child : children)
  {
    if (mNodes.at(child).name == name)
    {
      return true;
    }
  }
  return false;
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
      return wxDataViewTextRenderer::GetDefaultType();
    } break;
    default: { return "string"; }
  }
}

void Snippets::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  auto index = toIndex(item);
  variant = mNodes.at(index).name;
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
  const std::string newName = value.GetString().ToStdString();
  auto index = toIndex(item);
  auto &node = mNodes.at(index);
  if (node.name == newName) { return true; }
  if (newName.empty()) { return false; }

  wxLogInfo("Renaming '%zu' to '%s'", index, newName);

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(newName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", newName, e.what());
    return false;
  }

  const std::string fullpath = fmt::format(
    "{}/{}",
    mNodes.at(node.parent).fullpath.c_str(),
    encoded
  );

  std::error_code ec;
  fs::rename(node.fullpath, fullpath, ec);
  if (ec)
  {
    wxLogError(
      "Failed to rename '%s' to '%s': %s",
      node.fullpath.c_str(),
      fullpath,
      ec.message()
    );
    return false;
  }

  node.name = newName;
  node.fullpath = fullpath;
  node.saved = false;
  save(index);
  ItemChanged(item);
  return true;

  return false;
}

bool Snippets::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
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

  auto index = toIndex(item);
  auto parent = mNodes.at(index).parent;
  return toItem(parent);
}

bool Snippets::IsContainer(
  const wxDataViewItem &item
) const {

  if (!item.IsOk())
  {
    return true;
  }

  const auto &node = mNodes.at(toIndex(item));
  return node.type == Node::Type::Folder;
}

unsigned int Snippets::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  Node::Index_t index = 0;

  if (parent.IsOk())
  {
    index = toIndex(parent);
  }

  const auto &node = mNodes.at(index);

  for (auto i : node.children)
  {
    array.Add(toItem(i));
  }
  return node.children.size();
}

Snippets::Node::Index_t Snippets::toIndex(const wxDataViewItem &item)
{
  return reinterpret_cast<Node::Index_t>(item.GetID());
}

wxDataViewItem Snippets::toItem(Node::Index_t index)
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}

Snippets::Node::Index_t Snippets::getNextIndex()
{
  return mNextAvailableIndex++;
}

void Snippets::saveAll()
{
  // Traverse backwards.
  // If we save a child, the parents are saved with it.
  for (Node::Index_t i = mNodes.size() - 1; i >= 0; --i)
  {
    if (!mNodes.at(i).saved)
    {
      save(i);
    }
  }
}

bool Snippets::save(Node::Index_t index)
{
  if (index == 0) { return true; }

  auto &node = mNodes.at(index);
  if (node.saved) { return true; }
  if (!save(node.parent)) { return false; }

  if (node.type == Node::Type::Snippet)
  {
    std::ofstream output(node.fullpath);
    if (!output.is_open())
    {
      wxLogError("Could not save '%s'", node.name);
      return false;
    }

    output << MQTT::Message::toJson(*node.message);
  }
  else if (node.type == Node::Type::Folder)
  {
    bool exists = fs::exists(node.fullpath);
    bool isDir = fs::is_directory(node.fullpath);

    wxLogDebug(
      "exists=%s, isDir=%s",
      (exists ? "yes" : "no"),
      (isDir ? "yes" : "no")
    );

    if (exists && !isDir && !fs::remove(node.fullpath))
    {
      wxLogWarning("Could not remove file %s", node.fullpath.c_str());
      return false;
    }

    if (!exists && !fs::create_directory(node.fullpath))
    {
      wxLogWarning(
        "Could not create directory: %s",
        node.fullpath.c_str()
      );
      return false;
    }
  }

  node.saved = true;

  return true;
}
