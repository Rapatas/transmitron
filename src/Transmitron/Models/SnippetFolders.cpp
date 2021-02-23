#include "SnippetFolders.hpp"

#include <wx/log.h>
#include <wx/artprov.h>

#define wxLOG_COMPONENT "models/snippetFolders"

using namespace Transmitron::Models;

SnippetFolders::SnippetFolders(const wxObjectDataPtr<Snippets> snippetsModel) :
  mSnippetsModel(std::move(snippetsModel))
{}

wxDataViewItem SnippetFolders::createFolder(
  wxDataViewItem requestedParent
) {
  wxDataViewItem parent = requestedParent;
  if (toIndex(requestedParent) == FakeRootId)
  {
    parent = wxDataViewItem(nullptr);
  }

  auto inserted = mSnippetsModel->createFolder(parent);
  if (inserted.IsOk())
  {
    ItemAdded(requestedParent, inserted);
  }

  return inserted;
}

wxDataViewItem SnippetFolders::insert(
  const std::string &name,
  std::shared_ptr<MQTT::Message> message,
  wxDataViewItem requestedParent
) {
  wxDataViewItem parent = requestedParent;
  if (toIndex(requestedParent) == FakeRootId)
  {
    parent = wxDataViewItem(nullptr);
  }

  auto inserted = mSnippetsModel->insert(name, message, parent);
  if (inserted.IsOk())
  {
    wxLogInfo("ready");
    ItemAdded(requestedParent, inserted);
    wxLogInfo("ok");
  }

  return inserted;
}

bool SnippetFolders::remove(wxDataViewItem item)
{
  if (toIndex(item) == FakeRootId)
  {
    return false;
  }

  auto parent = GetParent(item);
  wxLogMessage("Removing %zu under %zu", toIndex(item), toIndex(parent));
  bool done = mSnippetsModel->remove(item);

  if (done)
  {
    wxLogMessage("Notifying");
    ItemDeleted(parent, item);
    wxLogMessage("done");
  }

  return done;
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
  if (col > Column::Max)
  {
    return mSnippetsModel->GetColumnType(col);
  }

  return "string";
}

void SnippetFolders::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  if (toIndex(item) == FakeRootId)
  {
    variant << wxDataViewIconText {
      "root",
      wxArtProvider::GetIcon(wxART_FOLDER)
    };
  }
  else
  {
    mSnippetsModel->GetValue(variant, item, col);
  }
}

bool SnippetFolders::SetValue(
  const wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) {
  if (!item.IsOk())
  {
    return false;
  }
  if (toIndex(item) == FakeRootId)
  {
    return false;
  }
  return mSnippetsModel->SetValue(value, item, col);
}

bool SnippetFolders::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
) const {
  return mSnippetsModel->IsEnabled(item, col);
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
  if (toIndex(item) == FakeRootId)
  {
    return true;
  }
  else
  {
    return mSnippetsModel->IsContainer(item);
  }
}

unsigned int SnippetFolders::GetChildren(
  const wxDataViewItem &requestedParent,
  wxDataViewItemArray &array
) const {

  wxLogInfo("GetChildren of %zu", toIndex(requestedParent));

  if (!requestedParent.IsOk())
  {
    array.Add(toItem(FakeRootId));
    wxLogInfo("  - %zu", FakeRootId);
    return 1;
  }

  wxDataViewItem parent = requestedParent;

  if (toIndex(requestedParent) == FakeRootId)
  {
    parent = wxDataViewItem(nullptr);
  }

  return mSnippetsModel->GetChildren(parent, array);
}

size_t SnippetFolders::toIndex(const wxDataViewItem &item)
{
  return reinterpret_cast<size_t>(item.GetID());
}

wxDataViewItem SnippetFolders::toItem(size_t index)
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}
