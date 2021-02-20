#include "SnippetDialog.hpp"
#include <wx/bmpbuttn.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace Transmitron::Widgets;

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

  auto newGroup = new wxButton(
    this,
    -1,
    "",
    wxDefaultPosition,
    wxSize(OptionsHeight, OptionsHeight)
  );
  newGroup->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR));

  wxDataViewColumn* const name = new wxDataViewColumn(
    L"name",
    new wxDataViewTextRenderer(),
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
  mSnippetsCtrl->AppendColumn(name);
  mSnippetsCtrl->AssociateModel(mSnippetFoldersModel.get());
  mSnippetsCtrl->SetFont(font);

  auto item = mSnippetFoldersModel->getRootItem();
  mSnippetsCtrl->Select(item);
  mSnippetsCtrl->Expand(item);

  auto groupText = new wxStaticText(this, -1, "Group:");

  auto groupSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  groupSizer->Add(groupText, 0, wxEXPAND | wxTOP, 7);
  groupSizer->AddStretchSpacer(1);
  groupSizer->Add(newGroup, 0, wxEXPAND);

  auto cancel = new wxButton(this, -1, "Cancel");
  auto save = new wxButton(this, -1, "Save");

  auto buttonSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  buttonSizer->AddStretchSpacer(1);
  buttonSizer->Add(cancel, 0, wxEXPAND | wxRIGHT, 5);
  buttonSizer->Add(save,   0, wxEXPAND);

  auto vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(filenameText,  0, wxEXPAND);
  vsizer->Add(mSnippetName,  0, wxEXPAND);
  vsizer->Add(groupSizer,    0, wxEXPAND | wxTOP, 5);
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
    wxEVT_LEFT_DOWN,
    &SnippetDialog::onSaveClicked,
    this
  );
  cancel->Bind(
    wxEVT_LEFT_DOWN,
    &SnippetDialog::onCancelClicked,
    this
  );
  newGroup->Bind(
    wxEVT_LEFT_DOWN,
    &SnippetDialog::onNewGroupClicked,
    this
  );
}

void SnippetDialog::onSaveClicked(wxMouseEvent &e)
{
  e.Skip();
}

void SnippetDialog::onCancelClicked(wxMouseEvent &e)
{

  e.Skip();
}

void SnippetDialog::onNewGroupClicked(wxMouseEvent &e)
{

  e.Skip();
}

void SnippetDialog::onKeyPressed(wxKeyEvent &e)
{
  if (e.GetKeyCode() == wxKeyCode::WXK_RETURN)
  {

  }
  else
  {
    e.Skip();
  }
}
