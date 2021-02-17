#include "SnippetFolders.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include <wx/log.h>
#include <fmt/format.h>
#include <cppcodec/base32_rfc4648.hpp>

#define wxLOG_COMPONENT "models/snippets"

namespace fs = std::filesystem;
using namespace Transmitron::Models;

SnippetFolders::SnippetFolders()
{
  mNodes.emplace_back();

  Node root {
    0,
    "root",
    {}
  };
  mNodes.push_back(std::move(root));
}

bool SnippetFolders::load(const wxObjectDataPtr<Snippets> snippetsModel)
{
  loadRecursive(toItem(0), snippetsModel);
  return true;
}

wxDataViewItem SnippetFolders::getRootItem() const
{
  return toItem(1);
}

void SnippetFolders::loadRecursive(
  const wxDataViewItem &parent,
  wxObjectDataPtr<Snippets> snippetsModel
) {
  auto currentParentIndex = mNodes.size() - 1;
  wxDataViewItemArray array;
  snippetsModel->GetChildren(parent, array);

  if (array.empty())
  {
    return;
  }

  for (const auto &item : array)
  {
    if (!snippetsModel->IsContainer(item))
    {
      continue;
    }

    wxVariant name;
    snippetsModel->GetValue(
      name,
      item,
      static_cast<unsigned>(Snippets::Column::Name)
    );
    Node newNode {
      currentParentIndex,
      name,
      {},
      item
    };
    mNodes.push_back(std::move(newNode));

    mNodes[currentParentIndex].children.push_back(mNodes.size() - 1);

    loadRecursive(item, snippetsModel);
  }
}

unsigned SnippetFolders::GetColumnCount() const
{
  return static_cast<unsigned>(Column::Max);
}

wxString SnippetFolders::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Name: {
      return wxDataViewTextRenderer::GetDefaultType();
    } break;
    default: { return "string"; }
  }
}

void SnippetFolders::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  auto index = toIndex(item);
  variant = mNodes.at(index).name;
}

bool SnippetFolders::SetValue(
  const wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) {
  return false;
}

bool SnippetFolders::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
) const {
  return true;
}

wxDataViewItem SnippetFolders::GetParent(
  const wxDataViewItem &item
) const {

  if (!item.IsOk())
  {
    return wxDataViewItem(0);
  }

  auto index = toIndex(item);

  auto parent = mNodes.at(index).parent;

  return toItem(parent);
}

bool SnippetFolders::IsContainer(
  const wxDataViewItem &item
) const {
  return true;
}

unsigned int SnippetFolders::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {

  if (!parent.IsOk())
  {
    array.Add(toItem(1));
    return 1;
  }

  auto index = toIndex(parent);

  const auto &node = mNodes.at(index);

  for (auto i : node.children)
  {
    array.Add(toItem(i));
  }
  return node.children.size();
}

SnippetFolders::Node::Index_t SnippetFolders::toIndex(const wxDataViewItem &item)
{
  return reinterpret_cast<Node::Index_t>(item.GetID());
}

wxDataViewItem SnippetFolders::toItem(Node::Index_t index)
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}

