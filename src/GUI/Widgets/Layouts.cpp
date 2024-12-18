#include "Layouts.hpp"

#include <iterator>

#include <wx/button.h>
#include <wx/event.h>

#include "Common/Log.hpp"
#include "GUI/Events/Layout.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Notifiers/Layouts.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Widgets;
using namespace GUI;

Layouts::Layouts(
  wxWindow *parent,
  wxWindowID id,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  wxAuiManager *auiMan,
  const ArtProvider &artProvider,
  int optionsHeight
) :
  wxPanel(parent, id),
  mOptionsHeight(optionsHeight),
  mLayoutsModel(layoutsModel),
  mAuiMan(auiMan),
  mArtProvider(artProvider),
  mSizer(new wxBoxSizer(wxHORIZONTAL)) //
{
  mLogger = Common::Log::create("Widgets::Layouts");

  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_ADDED, &Layouts::onLayoutAdded, this);
  notifier->Bind(Events::LAYOUT_REMOVED, &Layouts::onLayoutRemoved, this);
  notifier->Bind(Events::LAYOUT_CHANGED, &Layouts::onLayoutChanged, this);

  const auto options = getNames();
  mCurrentSelection = Models::Layouts::getDefault();
  const auto currentValue = mLayoutsModel->getName(mCurrentSelection);

  mLayoutsLocked = new wxComboBox(
    this,
    -1,
    currentValue,
    wxDefaultPosition,
    wxDefaultSize,
    options,
    wxCB_READONLY
  );

  mLayoutsEdit = new wxComboBox(
    this,
    -1,
    currentValue,
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
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mSave->SetBitmap(mArtProvider.bitmap(Icon::Save));

  mSizer->Add(mLayoutsLocked, 0, wxEXPAND);
  mSizer->Add(mLayoutsEdit, 0, wxEXPAND);
  mSizer->Add(mSave, 0, wxEXPAND);

  mLayoutsEdit->Bind(wxEVT_COMBOBOX, &Layouts::onLayoutEditSelected, this);
  mLayoutsEdit->Bind(wxEVT_TEXT_ENTER, &Layouts::onLayoutEditEnter, this);
  mLayoutsEdit->Bind(wxEVT_KILL_FOCUS, &Layouts::onLayoutEditLostFocus, this);
  mLayoutsLocked->Bind(wxEVT_COMBOBOX, &Layouts::onLayoutLockedSelected, this);
  mSave->Bind(wxEVT_BUTTON, &Layouts::onLayoutSaveClicked, this);

  SetSizer(mSizer);
}

bool Layouts::setSelectedLayout(const std::string &layoutName) {
  const auto item = mLayoutsModel->findItemByName(layoutName);

  if (!item.IsOk()) {
    mLogger->warn("Could not find perspective '{}'", layoutName);
    return false;
  }

  mLayoutsEdit->SetValue(layoutName);
  mLayoutsLocked->SetValue(layoutName);
  resize();
  onLayoutSelected(layoutName);

  return true;
}

void Layouts::onLayoutSaveClicked(wxCommandEvent & /* event */) {
  const auto perspective = mAuiMan->SavePerspective().ToStdString();
  const auto item = mLayoutsModel->create(perspective);

  if (!item.IsOk()) {
    mLogger->warn("Could not store perspective");
    return;
  }

  mCurrentSelection = item;
}

void Layouts::onLayoutEditEnter(wxCommandEvent & /* event */) {
  const auto wxs = mLayoutsEdit->GetValue();
  const wxVariant value = wxs;

  const auto saved = mLayoutsModel->SetValue(
    value,
    mCurrentSelection,
    static_cast<unsigned>(Models::Layouts::Column::Name)
  );

  if (!saved) { return; }

  mLayoutsEdit->SetValue(value.GetString());
  mLayoutsLocked->SetValue(value.GetString());
  resize();
}

void Layouts::onLayoutEditSelected(wxCommandEvent & /* event */) {
  onLayoutSelected(mLayoutsEdit->GetValue().ToStdString());
}

void Layouts::onLayoutEditLostFocus(wxFocusEvent & /* event */) {
  const auto value = mLayoutsModel->getName(mCurrentSelection);
  mLayoutsEdit->SetValue(value);
  mLayoutsLocked->SetValue(value);
  resize();
}

void Layouts::onLayoutLockedSelected(wxCommandEvent & /* event */) {
  onLayoutSelected(mLayoutsLocked->GetValue().ToStdString());
}

void Layouts::onLayoutSelected(const std::string &value) {
  const auto item = mLayoutsModel->findItemByName(value);
  if (!item.IsOk()) { return; }

  mCurrentSelection = item;
  const auto perspective = mLayoutsModel->getPerspective(item);

  auto *event = new Events::Layout(Events::LAYOUT_SELECTED);
  event->setPerspective(perspective);
  event->setItem(item);
  wxQueueEvent(this, event);
}

void Layouts::onLayoutAdded(Events::Layout &event) {
  const auto item = event.getItem();
  const auto prevValue = mLayoutsLocked->GetValue();

  const auto value = mLayoutsModel->getName(item);

  auto options = getNames();
  mLayoutsEdit->Set(options);
  mLayoutsLocked->Set(options);

  if (item == mCurrentSelection) {
    mLayoutsLocked->SetValue(value);
    mLayoutsEdit->SetValue(value);

    mLayoutsLocked->Hide();
    mLayoutsEdit->Show();
    mSizer->Layout();
    auto *event = new Events::Layout(Events::LAYOUT_RESIZED);
    wxQueueEvent(this, event);

    mLayoutsEdit->SelectAll();
    mLayoutsEdit->SetFocus();
  } else {
    mLayoutsEdit->SetValue(prevValue);
    mLayoutsLocked->SetValue(prevValue);
    resize();
  }
}

void Layouts::onLayoutRemoved(Events::Layout & /* event */) {
  const auto editval = mLayoutsEdit->GetValue();
  const auto lockval = mLayoutsLocked->GetValue();

  auto options = getNames();

  mLayoutsEdit->Set(options);
  mLayoutsLocked->Set(options);

  mLayoutsEdit->SetValue(editval);
  mLayoutsLocked->SetValue(lockval);
  resize();
}

void Layouts::onLayoutChanged(Events::Layout & /* event */) {
  auto options = getNames();

  mLayoutsEdit->Set(options);
  mLayoutsLocked->Set(options);

  wxVariant value;
  mLayoutsModel->GetValue(
    value,
    mCurrentSelection,
    static_cast<unsigned>(Models::Layouts::Column::Name)
  );

  mLayoutsEdit->SetValue(value.GetString());
  mLayoutsLocked->SetValue(value.GetString());
  resize();
}

wxArrayString Layouts::getNames() const {
  wxArrayString result;
  const wxDataViewItem parent(nullptr);
  wxDataViewItemArray children;
  mLayoutsModel->GetChildren(parent, children);
  for (const auto &child : children) {
    wxVariant value;
    mLayoutsModel->GetValue(
      value,
      child,
      static_cast<unsigned>(Models::Layouts::Column::Name)
    );
    result.push_back(value.GetString());
  }
  return result;
}

void Layouts::resize() {
  mLayoutsEdit->Hide();

  mLayoutsLocked->Hide();
  mLayoutsLocked->Show();

  mSizer->Layout();

  auto *event = new Events::Layout(Events::LAYOUT_RESIZED);
  wxQueueEvent(this, event);
}
