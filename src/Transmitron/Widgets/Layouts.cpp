#include "Layouts.hpp"
#include "Transmitron/Events/Layout.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Notifiers/Layouts.hpp"

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/log.h>

#define wxLOG_COMPONENT "Widgets/Layout" // NOLINT

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
  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_ADDED, &Layouts::onLayoutAdded, this);
  notifier->Bind(Events::LAYOUT_REMOVED, &Layouts::onLayoutRemoved, this);
  notifier->Bind(Events::LAYOUT_CHANGED, &Layouts::onLayoutChanged, this);

  const auto options = getNames();
  mCurrentSelection = mLayoutsModel->getDefault();

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
  mLayoutsEdit->Hide();

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
  mLayoutsEdit->Show();
  mLayoutsEdit->SetValue(mLayoutsModel->getUniqueName());

  mLayoutsLocked->Hide();
  mSizer->Layout();

  mLayoutsEdit->SelectAll();
  mLayoutsEdit->SetFocus();
}

void Layouts::onLayoutEditEnter(wxCommandEvent &/* event */)
{
  const auto value = mLayoutsEdit->GetValue().ToStdString();

  const auto perspective = mAuiMan.SavePerspective().ToStdString();
  const auto item = mLayoutsModel->create(value, perspective);

  if (!item.IsOk())
  {
    wxLogWarning("Could not load perspective");
    return;
  }

  mPendingSave = true;
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
  wxLogInfo("onLayoutSelected(%s)", value);

  const auto layoutOpt = mLayoutsModel->getLayout(value);
  if (!layoutOpt.has_value())
  {
    return;
  }

  const auto item = mLayoutsModel->getItem(value);
  if (!item.IsOk())
  {
    return;
  }

  mCurrentSelection = item;

  const auto &perspective = layoutOpt.value();

  auto *e = new Events::Layout(Events::LAYOUT_SELECTED);
  e->setPerspective(perspective);
  e->setItem(item);
  wxQueueEvent(this, e);
}

void Layouts::onLayoutAdded(Events::Layout &event)
{
  wxLogInfo("onLayoutAdded");
  const auto item = event.getItem();

  wxVariant value;
  mLayoutsModel->GetValue(value, item, 0);

  mLayoutsEdit->Append(value.GetString());
  mLayoutsLocked->Append(value.GetString());

  if (mPendingSave)
  {
    mLayoutsLocked->SetValue(value);
    mLayoutsEdit->SetValue(value);
    mLayoutsEdit->Hide();
    mLayoutsLocked->Show();
    mSizer->Layout();
    mPendingSave = false;
  }
}

void Layouts::onLayoutRemoved(Events::Layout &/* event */)
{
  wxLogInfo("onLayoutRemoved");
  const auto newOptions = getNames();

  const auto editval = mLayoutsEdit->GetValue();
  const auto lockval = mLayoutsLocked->GetValue();

  mLayoutsEdit->Set(newOptions);
  mLayoutsLocked->Set(newOptions);

  mLayoutsEdit->SetValue(editval);
  mLayoutsLocked->SetValue(lockval);
}

void Layouts::onLayoutChanged(Events::Layout &/* event */)
{
  wxLogInfo("onLayoutChanged");
  const auto newOptions = getNames();

  mLayoutsEdit->Set(newOptions);
  mLayoutsLocked->Set(newOptions);

  wxVariant value;
  mLayoutsModel->GetValue(value, mCurrentSelection, (unsigned)Models::Layouts::Column::Name);

  mLayoutsEdit->SetValue(value.GetString());
  mLayoutsLocked->SetValue(value.GetString());
}

wxArrayString Layouts::getNames() const
{
  wxArrayString result;
  wxLogInfo("Currently %d", mLayoutsModel->GetCount());
  for (unsigned i = 0; i < mLayoutsModel->GetCount(); ++i)
  {
    wxVariant value;
    mLayoutsModel->GetValueByRow(value, i, (unsigned)Models::Layouts::Column::Name);
    result.push_back(value.GetString());
  }
  return result;
}
