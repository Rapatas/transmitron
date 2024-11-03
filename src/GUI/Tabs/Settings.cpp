#include "Settings.hpp"

#include <wx/button.h>
#include <wx/colour.h>
#include <wx/listctrl.h>
#include <wx/propgrid/propgrid.h>
#include <wx/wx.h>

#include "Common/Log.hpp"
#include "GUI/Events/Connection.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Models/Profiles.hpp"
#include "GUI/Notifiers/Layouts.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Tabs;
using namespace GUI;

constexpr size_t SettingsWidth = 700;
constexpr size_t SettingsHeight = 500;
constexpr size_t Margin = 10;
constexpr size_t SectionsWidth = 100;
constexpr size_t SubSectionWidth = 250;

Settings::Settings(
  wxWindow *parent,
  const ArtProvider &artProvider,
  wxFontInfo labelFont,
  int optionsHeight,
  const wxObjectDataPtr<Models::Profiles> &profilesModel,
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
  mOptionsHeight(optionsHeight),
  mArtProvider(artProvider),
  mProfilesModel(profilesModel),
  mLayoutsModel(layoutsModel) //
{
  mLogger = Common::Log::create("GUI::Settings");

  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_ADDED, &Settings::onLayoutAdded, this);
  notifier->Bind(Events::LAYOUT_REMOVED, &Settings::onLayoutRemoved, this);
  notifier->Bind(Events::LAYOUT_CHANGED, &Settings::onLayoutChanged, this);

  auto *master = new wxPanel(this);
  master->SetMinSize(wxSize(SettingsWidth, SettingsHeight));

  mSections = new wxListCtrl(
    master,
    wxID_ANY,
    wxDefaultPosition,
    wxSize(SectionsWidth, -1),
    wxLC_LIST | wxLC_SINGLE_SEL
  );
  mSections->InsertItem(0, "Layouts");
  mSections->InsertItem(1, "Profiles");
  mSections->Bind(wxEVT_LIST_ITEM_SELECTED, &Settings::onSectionSelected, this);

  auto *labelSections = new wxStaticText(master, wxID_ANY, "Settings");
  labelSections->SetMinSize(wxSize(-1, mOptionsHeight));
  labelSections->SetFont(mLabelFont);

  auto *lsizer = new wxBoxSizer(wxVERTICAL);
  lsizer->Add(labelSections, 0, wxEXPAND);
  lsizer->Add(mSections, 1, wxEXPAND);

  setupLayouts(master);
  setupProfiles(master);

  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  mSectionSizer = new wxBoxSizer(wxVERTICAL);
  mSectionSizer->Add(mLayouts, 1, wxEXPAND);
  mSectionSizer->Add(mProfiles, 1, wxEXPAND);
  mSectionSizer->AddSpacer(0);

  mSectionSizer->Hide(mProfiles);
  mSectionSizer->Hide(mLayouts);

  auto *msizer = new wxBoxSizer(wxHORIZONTAL);
  msizer->Add(lsizer, 0, wxEXPAND);
  msizer->AddSpacer(Margin);
  msizer->Add(mSectionSizer, 1, wxEXPAND);
  master->SetSizer(msizer);

  auto *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->AddStretchSpacer(1);
  sizer->Add(master, 0);
  sizer->AddStretchSpacer(1);
  SetSizer(sizer);

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Settings::onContextSelected, this);
}

void Settings::createProfile() {
  const auto parent = wxDataViewItem(nullptr);
  const auto item = mProfilesModel->createProfile(parent);
  selectProfile(item);
}

void Settings::selectProfile(wxDataViewItem profile) {
  mSections->SetItemState(1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  mProfileDelete->Enable();
  mProfilesCtrl->Select(profile);
  mProfilesCtrl->EnsureVisible(profile);
  mProfileOptionsSizer->Show(mProfileOptions);
  mProfileOptionsSizer->Layout();
  propertyGridFill(
    mProfilesModel->getBrokerOptions(profile),
    mProfilesModel->getClientOptions(profile)
  );

  allowSaveProfile();
}

void Settings::setupLayouts(wxPanel *parent) {
  auto *renderer = new wxDataViewTextRenderer(
    wxDataViewTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mLayoutColumnName = new wxDataViewColumn(
    L"name",
    renderer,
    static_cast<unsigned>(Models::Layouts::Column::Name),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mLayouts = new wxPanel(parent);

  auto *label = new wxStaticText(mLayouts, wxID_ANY, "Layouts");
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
  mLayoutsCtrl
    ->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &Settings::onLayoutsContext, this);
  mLayoutsCtrl
    ->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Settings::onLayoutSelected, this);
  mLayoutsCtrl
    ->Bind(wxEVT_DATAVIEW_ITEM_START_EDITING, &Settings::onLayoutsEdit, this);

  mLayoutDelete = new wxButton(
    mLayouts,
    wxID_ANY,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mLayoutDelete->SetToolTip("Delete selected layout");
  mLayoutDelete->SetBitmap(mArtProvider.bitmap(Icon::Delete));
  mLayoutDelete->Disable();
  mLayoutDelete->Bind(wxEVT_BUTTON, &Settings::onLayoutsDelete, this);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(label, 0, wxEXPAND);
  hsizer->AddStretchSpacer(1);
  hsizer->Add(mLayoutDelete, 0, wxEXPAND);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(hsizer, 0, wxEXPAND);
  vsizer->Add(mLayoutsCtrl, 1, wxEXPAND);
  vsizer->SetMinSize(SubSectionWidth, -1);

  auto *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(vsizer, 0, wxEXPAND);
  mLayouts->SetSizer(sizer);
}

void Settings::setupProfiles(wxPanel *parent) {
  mProfiles = new wxPanel(parent);

  auto *renderer = new wxDataViewIconTextRenderer(
    wxDataViewIconTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mProfileColumnName = new wxDataViewColumn(
    L"Name",
    renderer,
    static_cast<unsigned>(Models::Profiles::Column::Name),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mProfilesCtrl = new wxDataViewCtrl(
    mProfiles,
    -1,
    wxDefaultPosition,
    wxSize(SubSectionWidth, 0),
    wxDV_NO_HEADER
  );
  mProfilesCtrl->AssociateModel(mProfilesModel.get());
  mProfilesCtrl->AppendColumn(mProfileColumnName);
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Settings::onProfileSelected,
    this
  );
  mProfilesCtrl
    ->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &Settings::onProfileContext, this);
  mProfilesCtrl
    ->Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, &Settings::onProfileDrag, this);
  mProfilesCtrl->Bind(wxEVT_DATAVIEW_ITEM_DROP, &Settings::onProfileDrop, this);
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE,
    &Settings::onProfileDropPossible,
    this
  );

  mProfilesCtrl->EnableDropTarget(wxDF_UNICODETEXT);
  mProfilesCtrl->EnableDragSource(wxDF_UNICODETEXT);

  auto *profileCreate = new wxButton(
    mProfiles,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  profileCreate->SetToolTip("Create a new profile");
  profileCreate->SetBitmap(mArtProvider.bitmap(Icon::NewProfile));
  profileCreate->Bind(wxEVT_BUTTON, &Settings::onButtonClickedNewProfile, this);

  mProfileDelete = new wxButton(
    mProfiles,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mProfileDelete->SetToolTip("Delete selected profile");
  mProfileDelete->SetBitmap(mArtProvider.bitmap(Icon::Delete));
  mProfileDelete->Bind(wxEVT_BUTTON, &Settings::onProfileDelete, this);
  mProfileDelete->Disable();

  setupProfileOptions(mProfiles);

  auto *label = new wxStaticText(mProfiles, wxID_ANY, "Profiles");
  label->SetFont(mLabelFont);

  auto *bsizer = new wxBoxSizer(wxHORIZONTAL);
  bsizer->Add(label, 1, wxEXPAND);
  bsizer->Add(profileCreate, 0, wxEXPAND);
  bsizer->Add(mProfileDelete, 0, wxEXPAND);

  auto *lsizer = new wxBoxSizer(wxVERTICAL);
  lsizer->Add(bsizer, 0, wxEXPAND);
  lsizer->Add(mProfilesCtrl, 1, wxEXPAND);

  mProfileOptionsSizer = new wxBoxSizer(wxVERTICAL);
  mProfileOptionsSizer->AddSpacer(mOptionsHeight);
  mProfileOptionsSizer->Add(mProfileOptions, 1, wxEXPAND);
  mProfileOptionsSizer->Hide(mProfileOptions);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(lsizer, 0, wxEXPAND);
  hsizer->AddSpacer(Margin);
  hsizer->Add(mProfileOptionsSizer, 1, wxEXPAND);
  mProfiles->SetSizer(hsizer);
}

void Settings::setupProfileOptions(wxPanel *parent) {
  mProfileOptions = new wxPanel(parent);

  mProfileGrid = new wxPropertyGrid(
    mProfileOptions,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxPG_SPLITTER_AUTO_CENTER
  );

  mProfileGrid->DedicateKey(WXK_UP);
  mProfileGrid->DedicateKey(WXK_DOWN);

  mProfileGrid->Enable(false);

  auto &pfp = mProfileProperties;
  auto &pfg = mProfileGrid;

  pfp.resize(Properties::Max);

  mGridCategoryBroker = new wxPropertyCategory("Broker");
  mProfileGrid->Append(mGridCategoryBroker);

  pfp.at(Properties::Hostname) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxStringProperty("Hostname", "", {})
  );
  pfp.at(Properties::Port
  ) = pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Port", "", {}));
  pfp.at(Properties::Username) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxStringProperty("Username", "", {})
  );
  pfp.at(Properties::Password) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxStringProperty("Password", "", {})
  );
  pfp.at(Properties::ConnectTimeout) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxUIntProperty("Connect Timeout (s)", "", {})
  );
  pfp.at(Properties::DisconnectTimeout) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxUIntProperty("Disconnect Timeout (s)", "", {})
  );
  pfp.at(Properties::MaxInFlight) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxUIntProperty("Max in flight", "", {})
  );
  pfp.at(Properties::KeepAlive) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxUIntProperty("Keep alive interval", "", {})
  );
  pfp.at(Properties::ClientId) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxStringProperty("Client ID", "", {})
  );
  pfp.at(Properties::AutoReconnect) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxBoolProperty("Auto Reconnect", "", {})
  );
  pfp.at(Properties::MaxReconnectRetries) = pfg->AppendIn(
    mGridCategoryBroker,
    new wxUIntProperty("Max Reconnect Retries", "", {})
  );

  mGridCategoryClient = new wxPropertyCategory("Client");
  mProfileGrid->Append(mGridCategoryClient);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfp.at(Properties::Layout) = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  mProfileGrid->Bind(wxEVT_PG_CHANGED, &Settings::onProfileGridChanged, this);
  mProfileGrid->Bind(wxEVT_PG_CHANGING, &Settings::onProfileGridChanged, this);
  mProfileGrid
    ->Bind(wxEVT_PG_LABEL_EDIT_BEGIN, &Settings::onProfileGridChanged, this);
  mProfileGrid->Bind(wxEVT_PG_SELECTED, &Settings::onProfileGridChanged, this);

  mSave = new wxButton(
    mProfileOptions,
    -1,
    "Save",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mSave->SetToolTip("Save changes to this profile");
  mSave->SetBitmap(mArtProvider.bitmap(Icon::Save));
  mSave->Enable(false);
  mSave->Bind(wxEVT_BUTTON, &Settings::onButtonClickedSave, this);

  mCancel = new wxButton(
    mProfileOptions,
    -1,
    "Cancel",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mCancel->SetToolTip("Reset this profile to last saved state");
  mCancel->SetBitmap(mArtProvider.bitmap(Icon::Cancel));
  mCancel->Enable(false);
  mCancel->Bind(wxEVT_BUTTON, &Settings::onButtonClickedCancel, this);

  mConnect = new wxButton(
    mProfileOptions,
    -1,
    "Connect",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mConnect->SetToolTip("Connect to this profile");
  mConnect->Enable(false);
  mConnect->Bind(wxEVT_BUTTON, &Settings::onButtonClickedConnect, this);
  mConnect->SetBitmap(mArtProvider.bitmap(Icon::Connect));

  mProfileButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
  mProfileButtonsSizer->SetMinSize(0, mOptionsHeight);
  mProfileButtonsSizer->AddStretchSpacer(1);
  mProfileButtonsSizer->Add(mConnect, 0, wxEXPAND);
  mProfileButtonsSizer->Add(mCancel, 0, wxEXPAND);
  mProfileButtonsSizer->Add(mSave, 0, wxEXPAND);
  mProfileButtonsSizer->Hide(mSave);
  mProfileButtonsSizer->Hide(mCancel);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(mProfileGrid, 1, wxEXPAND);
  sizer->Add(mProfileButtonsSizer, 0, wxEXPAND);
  mProfileOptions->SetSizer(sizer);

  propertyGridClear();
}

void Settings::onLayoutsContext(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  if (!item.IsOk()) {
    event.Skip();
    return;
  }

  if (!Models::Layouts::isDeletable(item)) {
    event.Skip();
    return;
  }

  wxMenu menu;

  auto *rename = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::LayoutsRename),
    "Rename"
  );
  rename->SetBitmap(mArtProvider.bitmap(Icon::Edit));
  menu.Append(rename);

  auto *del = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::LayoutsDelete),
    "Delete"
  );
  del->SetBitmap(mArtProvider.bitmap(Icon::Delete));
  menu.Append(del);

  PopupMenu(&menu);
}

void Settings::onContextSelected(wxCommandEvent &event) {
  switch (static_cast<ContextIDs>(event.GetId())) {
    case ContextIDs::LayoutsDelete: onLayoutsDelete(event); break;
    case ContextIDs::LayoutsRename: onLayoutsRename(event); break;
    case ContextIDs::ProfilesDelete: onProfileDelete(event); break;
    case ContextIDs::ProfilesNewFolder: onProfileNewFolder(event); break;
    case ContextIDs::ProfilesNewProfile: onProfileNewProfile(event); break;
    case ContextIDs::ProfilesRename: onProfileRename(event); break;
  }
  event.Skip();
}

void Settings::onLayoutsDelete(wxCommandEvent & /* event */) {
  mLogger->info("Requesting delete");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  if (!Models::Layouts::isDeletable(item)) { return; }
  mLayoutsModel->remove(item);
}

void Settings::onLayoutsRename(wxCommandEvent & /* event */) {
  mLogger->info("Requesting rename");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mLayoutsCtrl->EditItem(item, mLayoutColumnName);
}

void Settings::onLayoutsEdit(wxDataViewEvent &event) {
  mLogger->info("Requesting edit");
  const auto item = event.GetItem();
  if (!Models::Layouts::isDeletable(item)) {
    event.Veto();
  } else {
    event.Skip();
  }
}

void Settings::onProfileContext(wxDataViewEvent &event) {
  wxMenu menu;

  if (event.GetItem().IsOk()) {
    auto *del = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesDelete),
      "Delete"
    );
    del->SetBitmap(mArtProvider.bitmap(Icon::Delete));
    menu.Append(del);

    auto *rename = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesRename),
      "Rename"
    );
    rename->SetBitmap(mArtProvider.bitmap(Icon::Edit));
    menu.Append(rename);
  }

  if (!event.GetItem().IsOk() || mProfilesModel->IsContainer(event.GetItem())) {
    auto *newProfile = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesNewProfile),
      "New Profile"
    );
    newProfile->SetBitmap(mArtProvider.bitmap(Icon::NewFile));
    menu.Append(newProfile);

    auto *newFolder = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesNewFolder),
      "New Folder"
    );
    newFolder->SetBitmap(mArtProvider.bitmap(Icon::NewDir));
    menu.Append(newFolder);
  }

  PopupMenu(&menu);
}

void Settings::onProfileDelete(wxCommandEvent & /* event */) {
  const auto item = mProfilesCtrl->GetSelection();
  mProfilesModel->remove(item);
  propertyGridClear();
}

void Settings::onProfileNewFolder(wxCommandEvent & /* event */) {
  auto parent = mProfilesCtrl->GetSelection();
  if (!mProfilesModel->IsContainer(parent)) {
    parent = mProfilesModel->GetParent(parent);
  }

  const auto item = mProfilesModel->createFolder(parent);
  if (!item.IsOk()) { return; }

  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
  mProfilesCtrl->EditItem(item, mProfileColumnName);
}

void Settings::onProfileNewProfile(wxCommandEvent & /* event */) {
  auto parent = mProfilesCtrl->GetSelection();
  if (!mProfilesModel->IsContainer(parent)) {
    parent = mProfilesModel->GetParent(parent);
  }

  const auto item = mProfilesModel->createProfile(parent);
  if (!item.IsOk()) { return; }

  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
  mProfilesCtrl->EditItem(item, mProfileColumnName);
}

void Settings::onProfileRename(wxCommandEvent & /* event */) {
  mLogger->info("Requesting rename");
  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mProfilesCtrl->EditItem(item, mProfileColumnName);
}

void Settings::onProfileSelected(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  if (!item.IsOk()) {
    mProfileDelete->Disable();
    mProfileOptionsSizer->Hide(mProfileOptions);
    mProfileOptionsSizer->Layout();
    event.Skip();
    return;
  }

  if (mProfilesModel->IsContainer(item)) {
    mProfileDelete->Enable();
    mProfileOptionsSizer->Hide(mProfileOptions);
    mProfileOptionsSizer->Layout();
    event.Skip();
    return;
  }

  mProfileDelete->Enable();
  mProfileOptionsSizer->Show(mProfileOptions);
  mProfileOptionsSizer->Layout();

  propertyGridFill(
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );
  event.Skip();
}

void Settings::onProfileDrag(wxDataViewEvent &event) {
  auto item = event.GetItem();
  mProfilesWasExpanded = mProfilesCtrl->IsExpanded(item);

  const void *id = item.GetID();
  uintptr_t message = 0;
  std::memcpy(&message, &id, sizeof(uintptr_t));
  auto *object = new wxTextDataObject(std::to_string(message));

  event.SetDataFormat(object->GetFormat());
  event.SetDataSize(object->GetDataSize());
  event.SetDataObject(object);

  // Required for windows, ignored on all else.
  event.SetDragFlags(wxDrag_AllowMove);

  event.Skip(false);
}

void Settings::onProfileDrop(wxDataViewEvent &event) {
  const auto target = event.GetItem();

  wxTextDataObject object;
  object
    .SetData(event.GetDataFormat(), event.GetDataSize(), event.GetDataBuffer());
  const uintptr_t message = std::stoul(object.GetText().ToStdString());
  void *id = nullptr;
  std::memcpy(&id, &message, sizeof(uintptr_t));
  auto item = wxDataViewItem(id);

  wxDataViewItem moved;

#ifdef WIN32

  const auto pIndex = event.GetProposedDropIndex();

  if (pIndex == -1) {
    if (!mProfilesModel->IsContainer(target)) {
      moved = mProfilesModel->moveAfter(item, target);
    } else {
      if (target.IsOk()) {
        moved = mProfilesModel->moveInsideFirst(item, target);
      } else {
        moved = mProfilesModel->moveInsideLast(item, target);
      }
    }
  } else {
    const auto index = (size_t)pIndex;
    moved = mProfilesModel->moveInsideAtIndex(item, target, index);
  }

#else

  if (mProfilesPossible.first) {
    if (mProfilesModel->IsContainer(target)) {
      moved = mProfilesModel->moveInsideFirst(item, target);
    } else {
      moved = mProfilesModel->moveAfter(item, target);
    }
  } else {
    if (target.IsOk()) {
      moved = mProfilesModel->moveBefore(item, target);
    } else {
      moved = mProfilesModel->moveInsideLast(item, target);
    }
  }

#endif // WIN32

  mProfilesPossible = {false, wxDataViewItem(nullptr)};

  if (!moved.IsOk()) { return; }

  mProfilesCtrl->Refresh();
  mProfilesCtrl->EnsureVisible(moved);
  if (mProfilesWasExpanded) { mProfilesCtrl->Expand(moved); }
  mProfilesCtrl->Select(moved);
}

void Settings::onProfileDropPossible(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  mProfilesPossible = {true, item};

#ifdef WIN32

  event.Skip(false);

#else

  // On Linux:
  // - This event contains the first element of the target directory.
  // - If this skips, the drop event target is the hovered item.
  // - If this does not skip, the drop event target is this event's target.

  event.Skip(true);

#endif // WIN32
}

void Settings::onProfileGridChanged(wxPropertyGridEvent &event) {
  allowSaveProfile();
  event.Skip();
}

void Settings::onButtonClickedNewProfile(wxCommandEvent &event) {
  (void)event;
  createProfile();
}

void Settings::onButtonClickedCancel(wxCommandEvent &event) {
  (void)event;

  propertyGridClear();

  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  propertyGridFill(
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );

  allowConnect();
}

void Settings::onButtonClickedSave(wxCommandEvent &event) {
  (void)event;

  const auto item = mProfilesCtrl->GetSelection();

  if (!item.IsOk()) { return; }

  const auto brokerOptions = brokerOptionsFromPropertyGrid();
  mProfilesModel->updateBrokerOptions(item, brokerOptions);

  const auto clientOptions = clientOptionsFromPropertyGrid();
  mProfilesModel->updateClientOptions(item, clientOptions);

  allowConnect();
}

void Settings::onButtonClickedConnect(wxCommandEvent &event) {
  (void)event;

  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto &brokerOptions = mProfilesModel->getBrokerOptions(item);

  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection(Events::CONNECTION_REQUESTED);
  connectionEvent->setProfile(item);
  wxQueueEvent(this, connectionEvent);
}

void Settings::allowSaveProfile() {
  mSave->Enable(true);
  mCancel->Enable(true);
  mProfileButtonsSizer->Show(mSave);
  mProfileButtonsSizer->Show(mCancel);

  mConnect->Enable(false);
  mProfileButtonsSizer->Hide(mConnect);

  mProfileButtonsSizer->Layout();
}

void Settings::allowConnect() {
  mSave->Enable(false);
  mCancel->Enable(false);
  mProfileButtonsSizer->Hide(mSave);
  mProfileButtonsSizer->Hide(mCancel);

  mConnect->Enable(true);
  mProfileButtonsSizer->Show(mConnect);

  mProfileButtonsSizer->Layout();
}

void Settings::propertyGridClear() {
  mSave->Enable(false);
  mConnect->Enable(false);
  mProfileGrid->Enable(false);

  auto &pfp = mProfileProperties;

  pfp.at(Properties::AutoReconnect)->SetValue({});
  pfp.at(Properties::ClientId)->SetValue({});
  pfp.at(Properties::ConnectTimeout)->SetValue({});
  pfp.at(Properties::DisconnectTimeout)->SetValue({});
  pfp.at(Properties::Hostname)->SetValue({});
  pfp.at(Properties::KeepAlive)->SetValue({});
  pfp.at(Properties::MaxInFlight)->SetValue({});
  pfp.at(Properties::MaxReconnectRetries)->SetValue({});
  pfp.at(Properties::Password)->SetValue({});
  pfp.at(Properties::Port)->SetValue({});
  pfp.at(Properties::Username)->SetValue({});
  pfp.at(Properties::Layout)->SetValue({});
}

void Settings::propertyGridFill(
  const MQTT::BrokerOptions &brokerOptions,
  const Types::ClientOptions &clientOptions
) {
  auto &pfp = mProfileProperties;

  pfp.at(Properties::AutoReconnect)->SetValue(brokerOptions.getAutoReconnect());
  pfp.at(Properties::ClientId)->SetValue(brokerOptions.getClientId());
  pfp.at(Properties::ConnectTimeout)
    ->SetValue(static_cast<int>(brokerOptions.getConnectTimeout().count()));
  pfp.at(Properties::DisconnectTimeout)
    ->SetValue(static_cast<int>(brokerOptions.getDisconnectTimeout().count()));
  pfp.at(Properties::Hostname)->SetValue(brokerOptions.getHostname());
  pfp.at(Properties::KeepAlive)
    ->SetValue(static_cast<int>(brokerOptions.getKeepAliveInterval().count()));
  pfp.at(Properties::MaxInFlight)
    ->SetValue(static_cast<int>(brokerOptions.getMaxInFlight()));
  pfp.at(Properties::MaxReconnectRetries)
    ->SetValue(static_cast<int>(brokerOptions.getMaxReconnectRetries()));
  pfp.at(Properties::Password)->SetValue(brokerOptions.getPassword());
  pfp.at(Properties::Port)
    ->SetValue(static_cast<uint16_t>(brokerOptions.getPort()));
  pfp.at(Properties::Username)->SetValue(brokerOptions.getUsername());

  (void)clientOptions;
  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();
  const bool hasValue = pfpLayout->StringToValue(
    layoutValue,
    clientOptions.getLayout()
  );
  if (hasValue) { pfpLayout->SetValue(layoutValue); }

  mSave->Enable(true);
  mConnect->Enable(true);
  mProfileGrid->Enable(true);
}

MQTT::BrokerOptions Settings::brokerOptionsFromPropertyGrid() const {
  const auto &pfp = mProfileProperties;

  const bool autoReconnect = pfp.at(Properties::AutoReconnect)->GetValue();
  const auto maxInFlight = static_cast<size_t>(
    pfp.at(Properties::MaxInFlight)->GetValue().GetInteger()
  );
  const auto maxReconnectRetries = static_cast<size_t>(
    pfp.at(Properties::MaxReconnectRetries)->GetValue().GetInteger()
  );
  const auto port = static_cast<uint16_t>(
    pfp.at(Properties::Port)->GetValue().GetInteger()
  );
  const auto connectTimeout = static_cast<size_t>(
    pfp.at(Properties::ConnectTimeout)->GetValue().GetInteger()
  );
  const auto disconnectTimeout = static_cast<size_t>(
    pfp.at(Properties::DisconnectTimeout)->GetValue().GetInteger()
  );
  const auto keepAliveInterval = static_cast<size_t>(
    pfp.at(Properties::KeepAlive)->GetValue().GetLong()
  );
  const auto clientId = pfp.at(Properties::ClientId)->GetValue();
  const auto hostname = pfp.at(Properties::Hostname)->GetValue();
  const auto password = pfp.at(Properties::Password)->GetValue();
  const auto username = pfp.at(Properties::Username)->GetValue();

  return MQTT::BrokerOptions{
    autoReconnect,
    maxInFlight,
    maxReconnectRetries,
    port,
    std::chrono::seconds(connectTimeout),
    std::chrono::seconds(disconnectTimeout),
    std::chrono::seconds(keepAliveInterval),
    clientId,
    hostname,
    password,
    username,
  };
}

Types::ClientOptions Settings::clientOptionsFromPropertyGrid() const {
  const auto &pfp = mProfileProperties;

  const auto &pfpLayout = pfp.at(Properties::Layout);
  const auto layoutValue = pfpLayout->GetValue();
  const auto layoutIndex = static_cast<size_t>(layoutValue.GetInteger());
  const auto layout = mLayoutsModel->getLabelArray()[layoutIndex];

  return Types::ClientOptions{
    layout.ToStdString(),
  };
}

void Settings::onLayoutAdded(Events::Layout & /* event */) { refreshLayouts(); }

void Settings::onLayoutRemoved(Events::Layout & /* event */) {
  refreshLayouts();
}

void Settings::onLayoutSelected(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  if (!item.IsOk()) {
    mLayoutDelete->Disable();
    event.Skip();
    return;
  }

  if (!Models::Layouts::isDeletable(item)) {
    mLayoutDelete->Disable();
    event.Skip();
    return;
  }

  mLayoutDelete->Enable();
  event.Skip();
}

void Settings::onLayoutChanged(Events::Layout & /* event */) {
  auto &pfp = mProfileProperties;
  auto &pfg = mProfileGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  const wxVariant layoutValue = pfpLayout->GetValue();

  pfg->RemoveProperty(pfpLayout);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  pfpLayout->SetValue(layoutValue);
}

void Settings::refreshLayouts() {
  auto &pfp = mProfileProperties;
  auto &pfg = mProfileGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  const auto layoutLabels = mLayoutsModel->getLabelArray();

  const auto selectedValue = [&]() -> std::optional<wxString> {
    if (!mProfileOptions->IsShown()) { return std::nullopt; }

    wxVariant layoutValue = pfpLayout->GetValue();
    auto layoutName = pfpLayout->ValueToString(layoutValue);
    mLogger->debug("Previous layout was: {}", layoutName.ToStdString());

    return layoutName;
  }();

  pfg->RemoveProperty(pfpLayout);

  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  if (selectedValue) {
    wxVariant newLayoutValue;
    const bool containsValue = pfpLayout->StringToValue(
      newLayoutValue,
      selectedValue.value()
    );
    if (containsValue) { pfpLayout->SetValue(newLayoutValue); }
  }
}

void Settings::onSectionSelected(wxListEvent &event) {
  const auto selected = event.GetIndex();
  if (selected == 0) {
    mSectionSizer->Hide(mProfiles);
    mSectionSizer->Show(mLayouts);
    mSectionSizer->Layout();
  } else {
    mSectionSizer->Show(mProfiles);
    mSectionSizer->Hide(mLayouts);
    mSectionSizer->Layout();
  }
}
