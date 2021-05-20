#include "Layouts.hpp"

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/log.h>

#include "Transmitron/Events/Layout.hpp"

#define wxLOG_COMPONENT "Layout" // NOLINT

using namespace Transmitron::Widgets;
using namespace Transmitron;

wxDEFINE_EVENT(Events::LAYOUT_SELECTED, Events::Layout); // NOLINT

Layouts::Layouts(
  wxWindow* parent,
  wxWindowID id,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  wxAuiManager &auiMan,
  size_t optionsHeight
) :
  wxPanel(parent, id),
  mOptionsHeight(optionsHeight),
  mLayoutsModel(layoutsModel),
  mAuiMan(auiMan)
{
  wxArrayString options = mLayoutsModel->getNames();
  mLayoutsLocked = new wxComboBox(
    this,
    -1,
    options.front(),
    wxDefaultPosition,
    wxDefaultSize,
    options,
    wxCB_READONLY
  );

  mLayoutsEdit = new wxComboBox(
    this,
    -1,
    options.front(),
    wxDefaultPosition,
    wxDefaultSize,
    options,
    wxTE_PROCESS_ENTER
  );
  mLayoutsEdit->Show(false);

  mSave = new wxButton(
    this,
    -1,
    "",
    wxDefaultPosition,
    wxSize((int)mOptionsHeight, (int)mOptionsHeight)
  );
  mSave->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

  mSizer = new wxBoxSizer(wxHORIZONTAL);
  mSizer->Add(mSave, 0, wxEXPAND);
  mSizer->Add(mLayoutsLocked, 0, wxEXPAND);
  mSizer->Add(mLayoutsEdit, 0, wxEXPAND);

  mLayoutsEdit->Bind(
    wxEVT_COMBOBOX,
    &Layouts::onLayoutEditSelected,
    this
  );
  mLayoutsEdit->Bind(
    wxEVT_TEXT_ENTER,
    &Layouts::onLayoutEditEnter,
    this
  );
  mLayoutsLocked->Bind(
    wxEVT_COMBOBOX,
    &Layouts::onLayoutLockedSelected,
    this
  );
  mSave->Bind(
    wxEVT_BUTTON,
    &Layouts::onLayoutSaveClicked,
    this
  );

  SetSizer(mSizer);
}

void Layouts::onLayoutSaveClicked(wxCommandEvent &/* event */)
{
  mLayoutsEdit->SetValue(mLayoutsModel->getUniqueName());

  mLayoutsEdit->Show(true);
  mLayoutsLocked->Show(false);
  mSizer->Layout();

  mLayoutsEdit->SelectAll();
  mLayoutsEdit->SetFocus();
}

void Layouts::onLayoutEditEnter(wxCommandEvent &/* event */)
{
  const auto value = mLayoutsEdit->GetValue().ToStdString();
  wxLogInfo("User created: %s", value);

  const auto perspective = mAuiMan.SavePerspective().ToStdString();
  const auto item = mLayoutsModel->create(value, perspective);

  if (!item.IsOk())
  {
    wxLogWarning("Could not load perspective");
    return;
  }

  mLayoutsEdit->Append(value);
  mLayoutsLocked->Append(value);

  mLayoutsLocked->SetValue(value);

  mLayoutsEdit->Hide();
  mLayoutsLocked->Show();
  mSizer->Layout();
}

void Layouts::onLayoutEditSelected(wxCommandEvent &/* event */)
{
  onLayoutSelected(mLayoutsEdit->GetValue().ToStdString());
}

void Layouts::onLayoutLockedSelected(wxCommandEvent &/* event */)
{
  onLayoutSelected(mLayoutsLocked->GetValue().ToStdString());
}

void Layouts::onLayoutSelected(const std::string &value)
{
  wxLogInfo("User selected: %s", value);

  const auto layoutOpt = mLayoutsModel->getLayout(value);
  if (!layoutOpt.has_value())
  {
    return;
  }

  const auto &perspective = layoutOpt.value();

  auto *e = new Events::Layout(Events::LAYOUT_SELECTED);
  e->setPerspective(perspective);
  wxQueueEvent(this, e);
}
