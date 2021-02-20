#include "SnippetFolders.hpp"

#include <wx/log.h>

#define wxLOG_COMPONENT "models/snippetFolders"

using namespace Transmitron::Models;

SnippetFolders::SnippetFolders(const wxObjectDataPtr<Snippets> snippetsModel) :
  mSnippetsModel(std::move(snippetsModel))
{}

bool SnippetFolders::insert(
  const MQTT::Message &message,
  wxDataViewItem parent
) {
  return true;
}

wxDataViewItem SnippetFolders::getRootItem() const
{
  return toItem(FakeRootId);
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
  if (toIndex(item) == FakeRootId)
  {
    variant = "root";
  }
  else
  {
    mSnippetsModel->GetValue(variant, item, col);
  }
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
  if (toIndex(item) == FakeRootId)
  {
    return wxDataViewItem(nullptr);
  }

  auto result = mSnippetsModel->GetParent(item);
  if (!result.IsOk())
  {
    return toItem(FakeRootId);
  }
  else
  {
    return result;
  }
}

bool SnippetFolders::IsContainer(
  const wxDataViewItem &item
) const {
  return true;
}

unsigned int SnippetFolders::GetChildren(
  const wxDataViewItem &requestedParent,
  wxDataViewItemArray &array
) const {

  if (!requestedParent.IsOk())
  {
    array.Add(toItem(FakeRootId));
    return 1;
  }

  wxDataViewItem parent = requestedParent;

  if (toIndex(requestedParent) == FakeRootId)
  {
    parent = wxDataViewItem(nullptr);
  }

  wxDataViewItemArray result;
  mSnippetsModel->GetChildren(parent, result);
  for (const auto &child : result)
  {
    if (mSnippetsModel->IsContainer(child))
    {
      array.Add(child);
    }
  }

  return array.size();
}

size_t SnippetFolders::toIndex(const wxDataViewItem &item)
{
  return reinterpret_cast<size_t>(item.GetID());
}

wxDataViewItem SnippetFolders::toItem(size_t index)
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}
