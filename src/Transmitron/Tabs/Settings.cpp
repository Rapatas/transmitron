#include "Settings.hpp"
#include "Transmitron/Models/Layouts.hpp"

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
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    "Settings"
  ),
  mLabelFont(std::move(labelFont)),
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
  auto *renderer = new wxDataViewTextRenderer(
    wxDataViewTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mLayoutColumnName = new wxDataViewColumn(
    L"name",
    renderer,
    (unsigned)Models::Layouts::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mLayouts = new wxPanel(this, -1);

  auto *label = new wxStaticText(mLayouts, -1, "Layouts");
  label->SetFont(mLabelFont);

  mLayoutsCtrl = new wxDataViewListCtrl(
    mLayouts,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mLayoutsCtrl->AppendColumn(mLayoutColumnName);
  mLayoutsCtrl->AssociateModel(mLayoutsModel.get());
  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Settings::onLayoutsContext,
    this
  );

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,        0, wxEXPAND);
  vsizer->Add(mLayoutsCtrl, 1, wxEXPAND);
  mLayouts->SetSizer(vsizer);

  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &Settings::onLayoutsEdit,
    this
  );
}

void Settings::onLayoutsContext(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    event.Skip();
    return;
  }

  if (mLayoutsModel->getDefault() == item)
  {
    event.Skip();
    return;
  }

  wxMenu menu;

  auto *rename = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::LayoutsRename,
    "Rename"
  );
  rename->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
  menu.Append(rename);

  auto *del = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::LayoutsDelete,
    "Delete"
  );
  del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(del);

  PopupMenu(&menu);
}

void Settings::onContextSelected(wxCommandEvent &event)
{
  switch ((ContextIDs)event.GetId())
  {
    case ContextIDs::LayoutsDelete: onLayoutsDelete(event); break;
    case ContextIDs::LayoutsRename: onLayoutsRename(event); break;
  }
  event.Skip();
}

void Settings::onLayoutsDelete(wxCommandEvent &/* event */)
{
  wxLogMessage("Requesting delete");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  if (mLayoutsModel->getDefault() == item) { return; }
  mLayoutsModel->remove(item);
}

void Settings::onLayoutsRename(wxCommandEvent &/* event */)
{
  wxLogMessage("Requesting rename");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mLayoutsCtrl->EditItem(item, mLayoutColumnName);
}

void Settings::onLayoutsEdit(wxDataViewEvent &event)
{
  wxLogMessage("Requesting edit");
  const auto item = event.GetItem();
  if (mLayoutsModel->getDefault() == item)
  {
    event.Veto();
  }
  else
  {
    event.Skip();
  }
}
