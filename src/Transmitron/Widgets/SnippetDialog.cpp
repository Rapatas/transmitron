#include "SnippetDialog.hpp"
#include <wx/bmpbuttn.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/log.h>

#define wxLOG_COMPONENT "SnippetDialog"

using namespace Transmitron::Widgets;
using namespace Transmitron::Models;

SnippetDialog::SnippetDialog(
  wxWindow *parent,
  wxWindowID id,
  wxObjectDataPtr<Models::SnippetFolders> snippetFoldersModel,
  size_t optionsHeight,
  std::shared_ptr<MQTT::Message> message
) :
  wxDialog(
    parent,
    id,
    "Save Snippet",
    wxDefaultPosition,
    wxSize(300, 300),
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP
  ),
  mSnippetFoldersModel(snippetFoldersModel),
  OptionsHeight(optionsHeight),
  mMessage(std::move(message))
{
  wxFont font(wxFontInfo(9).FaceName("Consolas"));

  auto filenameText = new wxStaticText(this, -1, "Name:");

  mSnippetName = new wxTextCtrl(this, -1, "");

  auto newFolder = new wxButton(
    this,
    -1,
    "",
    wxDefaultPosition,
    wxSize(OptionsHeight, OptionsHeight)
  );
  newFolder->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR));

  auto renderer = new wxDataViewIconTextRenderer(
    wxDataViewIconTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mColumns.at(SnippetFolders::Column::Name) = new wxDataViewColumn(
    L"name",
    renderer,
    (unsigned)Models::Snippets::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  mSnippetsCtrl = new wxDataViewListCtrl(
    this,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mSnippetsCtrl->AppendColumn(mColumns.at(SnippetFolders::Column::Name));
  mSnippetsCtrl->AssociateModel(mSnippetFoldersModel.get());
  mSnippetsCtrl->SetFont(font);
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &SnippetDialog::onCtrlEdit,
    this
  );

  auto item = mSnippetFoldersModel->getRootItem();
  mSnippetsCtrl->Select(item);
  mSnippetsCtrl->Expand(item);

  auto folderText = new wxStaticText(this, -1, "Folder:");

  auto folderSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  folderSizer->Add(folderText, 0, wxEXPAND | wxTOP, 7);
  folderSizer->AddStretchSpacer(1);
  folderSizer->Add(newFolder, 0, wxEXPAND);

  auto cancel = new wxButton(this, -1, "Cancel");
  auto save = new wxButton(this, -1, "Save");

  auto buttonSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  buttonSizer->AddStretchSpacer(1);
  buttonSizer->Add(cancel, 0, wxEXPAND | wxRIGHT, 5);
  buttonSizer->Add(save,   0, wxEXPAND);

  auto vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(filenameText,  0, wxEXPAND);
  vsizer->Add(mSnippetName,  0, wxEXPAND);
  vsizer->Add(folderSizer,    0, wxEXPAND | wxTOP, 5);
  vsizer->Add(mSnippetsCtrl, 1, wxEXPAND);
  vsizer->Add(buttonSizer,   0, wxEXPAND | wxTOP, 5);

  auto wrapper = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wrapper->Add(vsizer, 1, wxEXPAND | wxALL, 10);

  SetSizer(wrapper);

  mSnippetName->Bind(
    wxEVT_KEY_DOWN,
    &SnippetDialog::onKeyPressed,
    this
  );
  save->Bind(
    wxEVT_LEFT_UP,
    &SnippetDialog::onSaveClicked,
    this
  );
  cancel->Bind(
    wxEVT_LEFT_UP,
    &SnippetDialog::onCancelClicked,
    this
  );
  newFolder->Bind(
    wxEVT_LEFT_UP,
    &SnippetDialog::onNewFolderClicked,
    this
  );
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &SnippetDialog::onCtrlContext,
    this
  );

  Bind(
    wxEVT_COMMAND_MENU_SELECTED,
    &SnippetDialog::onContextSelected,
    this
  );
}

void SnippetDialog::onSaveClicked(wxMouseEvent &e)
{
  doSave();
  e.Skip();
}

void SnippetDialog::onCancelClicked(wxMouseEvent &e)
{
  Destroy();
  e.Skip();
}

void SnippetDialog::onNewFolderClicked(wxMouseEvent &e)
{
  auto item = mSnippetsCtrl->GetSelection();
  if (!mSnippetFoldersModel->IsContainer(item))
  {
    item = mSnippetFoldersModel->GetParent(item);
  }
  auto inserted = mSnippetFoldersModel->createFolder(item);
  handleInserted(inserted);
  e.Skip();
}

void SnippetDialog::onKeyPressed(wxKeyEvent &e)
{
  if (e.GetKeyCode() == wxKeyCode::WXK_RETURN)
  {
    doSave();
  }
  else
  {
    e.Skip();
  }
}

void SnippetDialog::onCtrlEdit(wxDataViewEvent &e)
{
  const bool isRootItem =
    e.GetItem().IsOk()
    && e.GetItem() == mSnippetFoldersModel->getRootItem();

  if (isRootItem || !mExplicitEditRequest)
  {
    e.Veto();
  }
  else
  {
    mExplicitEditRequest = false;
    e.Skip();
  }
}

void SnippetDialog::onCtrlContext(wxDataViewEvent &e)
{
  auto item = e.GetItem();
  if (!item.IsOk()) { return; }

  mSnippetsCtrl->Select(item);

  wxMenu menu;

  auto newFolder = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::New,
    "New Folder"
  );
  newFolder->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR));
  menu.Append(newFolder);

  if (item != mSnippetFoldersModel->getRootItem())
  {
    auto rename = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::Rename,
      "Rename"
    );
    rename->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
    menu.Append(rename);

    auto del = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::Delete,
      "Delete"
    );
    del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
    menu.Append(del);
  }

  PopupMenu(&menu);
}

void SnippetDialog::onContextSelected(wxCommandEvent &e)
{
  switch(static_cast<ContextIDs>(e.GetId()))
  {
    case ContextIDs::Rename: {
      onContextSelectedRename(e);
    } break;
    case ContextIDs::Delete: {
      onContextSelectedDelete(e);
    } break;
    case ContextIDs::New: {
      onContextSelectedNew(e);
    } break;
  }
  e.Skip();
}

void SnippetDialog::onContextSelectedRename(wxCommandEvent &e)
{
  wxLogMessage("Requesting rename");
  mExplicitEditRequest = true;
  auto item = mSnippetsCtrl->GetSelection();
  mSnippetsCtrl->EditItem(item, mColumns.at(SnippetFolders::Column::Name));
}

void SnippetDialog::onContextSelectedDelete(wxCommandEvent &e)
{
  wxLogMessage("Requesting delete");

  auto item = mSnippetsCtrl->GetSelection();
  bool done = mSnippetFoldersModel->remove(item);

  if (done)
  {
    wxLogMessage("Removed!");
    auto root = mSnippetFoldersModel->getRootItem();
    wxLogMessage("Got root");
    mSnippetsCtrl->Select(root);
    wxLogMessage("Selected");
  }
}

void SnippetDialog::onContextSelectedNew(wxCommandEvent &e)
{
  wxLogMessage("Requesting new folder");
  auto item = mSnippetsCtrl->GetSelection();
  if (!mSnippetFoldersModel->IsContainer(item))
  {
    wxLogInfo("Selecting parent");
    item = mSnippetFoldersModel->GetParent(item);
  }
  auto inserted = mSnippetFoldersModel->createFolder(item);
  handleInserted(inserted);
}

void SnippetDialog::handleInserted(wxDataViewItem &inserted)
{
  wxLogInfo("handleInserted");
  if (!inserted.IsOk())
  {
    wxLogInfo("failed");
    return;
  }
  wxLogInfo("looks ok");

  mSnippetsCtrl->Select(inserted);
  mSnippetsCtrl->EnsureVisible(inserted);
  auto nameColumn = mColumns.at(SnippetFolders::Column::Name);
  mExplicitEditRequest = true;
  mSnippetsCtrl->EditItem(inserted, nameColumn);
}

bool SnippetDialog::doSave()
{
  auto name = mSnippetName->GetValue().ToStdString();
  if (name.empty())
  {
    wxLogWarning("Can not save with blank name");
    return false;
  }

  auto item = mSnippetsCtrl->GetSelection();
  if (!mSnippetFoldersModel->IsContainer(item))
  {
    wxLogInfo("Selecting parent");
    item = mSnippetFoldersModel->GetParent(item);
  }
  auto inserted = mSnippetFoldersModel->insert(name, mMessage, item);
  wxLogInfo("Inserted: %s", (inserted.IsOk() ? "yes" : "no"));
  if (inserted.IsOk())
  {
    Destroy();
  }
  return inserted;
}
