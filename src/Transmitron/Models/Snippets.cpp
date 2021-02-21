#include "Snippets.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include <wx/log.h>
#include <fmt/format.h>
#include <cppcodec/base32_rfc4648.hpp>

#define wxLOG_COMPONENT "models/snippets"

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
    nullptr
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
      "Could not create snippets directory: %s",
      mSnippetsDir
    );
    return false;
  }

  loadRecursive(mSnippetsDir);

  return true;
}

void Snippets::loadRecursive(const std::filesystem::path &snippetsDir)
{
  Node::Index_t currentParentIndex = mNodes.size() - 1;

  for (const auto &entry : fs::directory_iterator(snippetsDir))
  {
    std::vector<uint8_t> decoded;
    try { decoded = cppcodec::base32_rfc4648::decode(entry.path().stem().u8string()); }
    catch (cppcodec::parse_error &e)
    {
      wxLogError("Could not decode '%s': %s", entry.path().u8string(), e.what());
      continue;
    }
    std::string name{decoded.begin(), decoded.end()};

    if (entry.status().type() == fs::file_type::directory)
    {
      Node newNode {
        currentParentIndex,
        name,
        Node::Type::Folder,
        {},
        nullptr
      };
      const auto newIndex = getNextIndex();
      mNodes.insert({newIndex, std::move(newNode)});
      mNodes[currentParentIndex].children.insert(newIndex);
      loadRecursive(entry.path());
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
        currentParentIndex,
        name,
        Node::Type::Snippet,
        {},
        std::move(message)
      };
      const auto newIndex = getNextIndex();
      mNodes.insert({newIndex, std::move(newNode)});
      mNodes[currentParentIndex].children.insert(newIndex);
    }
  }
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
  const wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) {
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

  if (index == 0)
  {
    return wxDataViewItem(nullptr);
  }

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
