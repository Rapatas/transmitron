#include "SnippetDialog.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace Transmitron::Widgets;

SnippetDialog::SnippetDialog(
  wxWindow *parent,
  wxWindowID id,
  wxObjectDataPtr<Models::SnippetFolders> snippetFoldersModel
) :
  wxDialog(
    parent,
    id,
    "Save Snippet",
    wxDefaultPosition,
    wxSize(300, 200),
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP
  ),
  mSnippetFoldersModel(snippetFoldersModel)
{

  size_t OptionsHeight = 26;


  wxDataViewColumn* const name = new wxDataViewColumn(
    L"name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Snippets::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );


  // Ctrl.

  mSnippetsCtrl = new wxDataViewListCtrl(
    this,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );

  mSnippetsCtrl->AppendColumn(name);
  mSnippetsCtrl->AssociateModel(mSnippetFoldersModel.get());

  wxFont font(wxFontInfo(9).FaceName("Consolas"));
  mSnippetsCtrl->SetFont(font);

  auto item = mSnippetFoldersModel->getRootItem();
  mSnippetsCtrl->Select(item);
  mSnippetsCtrl->Expand(item);

  auto groupText = new wxStaticText(
    this,
    -1,
    "Group",
    wxDefaultPosition,
    wxSize(50, OptionsHeight),
    wxALIGN_RIGHT
  );

  auto ctrlSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  ctrlSizer->Add(groupText, 0, wxEXPAND | wxRIGHT, 5);
  ctrlSizer->Add(mSnippetsCtrl, 1, wxEXPAND);


  // Name.

  mSnippetName = new wxTextCtrl(
    this,
    -1,
    "",
    wxDefaultPosition,
    wxSize(0, OptionsHeight)
  );

  auto filenameText = new wxStaticText(
    this,
    -1,
    "Name",
    wxDefaultPosition,
    wxSize(50, 0),
    wxALIGN_RIGHT
  );

  auto nameSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  nameSizer->Add(filenameText, 0, wxEXPAND | wxRIGHT, 5);
  nameSizer->Add(mSnippetName, 1, wxEXPAND);



  // Buttons.

  auto cancel = new wxButton(this, -1, "Cancel");
  auto save = new wxButton(this, -1, "Save");

  auto buttonSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  buttonSizer->AddStretchSpacer(1);
  buttonSizer->Add(cancel, 0, wxEXPAND | wxRIGHT, 10);
  buttonSizer->Add(save, 0, wxEXPAND);


  // Master.

  auto master =  new wxBoxSizer(wxOrientation::wxVERTICAL);
  master->Add(nameSizer, 0, wxEXPAND);
  master->Add(ctrlSizer, 1, wxEXPAND);
  master->Add(buttonSizer, 0, wxEXPAND | wxTOP , 10);

  auto wrapper = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wrapper->Add(master, 1, wxEXPAND | wxALL, 10);


  SetSizer(wrapper);
}
