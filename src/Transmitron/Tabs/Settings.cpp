#include "Settings.hpp"

#include <wx/button.h>
#include <wx/wx.h>
#include <wx/artprov.h>

#define wxLOG_COMPONENT "Settings" // NOLINT

using namespace Transmitron::Tabs;
using namespace Transmitron;

Settings::Settings(
  wxWindow *parent,
  wxFontInfo labelFont,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel
) :
  wxPanel(parent),
  labelFont(std::move(labelFont)),
  mLayoutsModel(layoutsModel)
{
  setupLayouts();

  auto *panel = new wxPanel(this, -1);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(mLayouts, 1, wxEXPAND);
  hsizer->Add(panel, 1, wxEXPAND);
  hsizer->Layout();

  this->SetSizer(hsizer);

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Settings::onContextSelected, this);
}

void Settings::setupLayouts()
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    "Name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Layouts::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mLayouts = new wxPanel(this, -1);

  auto *label = new wxStaticText(mLayouts, -1, "Layouts");
  label->SetFont(labelFont);

  mLayoutsCtrl = new wxDataViewCtrl(mLayouts, -1);
  mLayoutsCtrl->AssociateModel(mLayoutsModel.get());
  mLayoutsCtrl->AppendColumn(name);
  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Settings::onLayoutsContext,
    this
  );

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,        0, wxEXPAND);
  vsizer->Add(mLayoutsCtrl, 1, wxEXPAND);
  mLayouts->SetSizer(vsizer);
}

void Settings::onLayoutsContext(wxDataViewEvent &e)
{
  if (!e.GetItem().IsOk())
  {
    e.Skip();
    return;
  }

  wxMenu menu;

  auto *del = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::LayoutsDelete,
    "Delete"
  );
  del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(del);

  PopupMenu(&menu);
}

void Settings::onContextSelected(wxCommandEvent &e)
{
  switch ((ContextIDs)e.GetId())
  {
    case ContextIDs::LayoutsDelete: onLayoutsDelete(e); break;
    case ContextIDs::LayoutsRename: onLayoutsRename(e); break;
  }
  e.Skip();
}

void Settings::onLayoutsDelete(wxCommandEvent & /* event */)
{
  wxLogMessage("Requesting delete");
  const auto item = mLayoutsCtrl->GetSelection();
  mLayoutsModel->remove(item);
}

void Settings::onLayoutsRename(wxCommandEvent & /* event */)
{
  wxLogMessage("Requesting rename");
  // const auto item = mLayoutsCtrl->GetSelection();
  // mLayoutsModel->remove(item);
}

