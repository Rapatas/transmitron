#include <wx/colour.h>
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>

#include "Common/Log.hpp"
#include "Settings.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Notifiers/Layouts.hpp"
#include "GUI/Events/Connection.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Tabs;
using namespace GUI;

constexpr size_t SettingsWidth = 600;
constexpr size_t SettingsHeight = 500;
constexpr size_t Margin = 10;
constexpr size_t SectionsWidth = 90;
constexpr size_t SubSectionWidth = 200;

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
  mLayoutsModel(layoutsModel)
{
  mLogger = Common::Log::create("GUI::Settings");

  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_ADDED,   &Settings::onLayoutAdded,   this);
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

void Settings::createProfile()
{
  const auto item = mProfilesModel->createProfile();
  selectProfile(item);
}

void Settings::selectProfile(wxDataViewItem profile)
{
  mSections->SetItemState(1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  mProfileDelete->Enable();
  mProfilesCtrl->Select(profile);
  mProfilesCtrl->EnsureVisible(profile);
  mProfileOptionsSizer->Show(mProfileOptions);
  mProfileOptionsSizer->Layout();
  propertyGridFill(
    mProfilesModel->getName(profile),
    mProfilesModel->getBrokerOptions(profile),
    mProfilesModel->getClientOptions(profile)
  );

  const auto *namePtr = mProfileProperties.at(Properties::Name);
  const bool focus = true;
  mProfileGrid->SelectProperty(namePtr, focus);

  allowSave();
}

void Settings::setupLayouts(wxPanel *parent)
{
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
  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Settings::onLayoutsContext,
    this
  );
  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Settings::onLayoutSelected,
    this
  );

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

  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &Settings::onLayoutsEdit,
    this
  );
}

void Settings::setupProfiles(wxPanel *parent)
{
  mProfiles = new wxPanel(parent);

  auto* const name = new wxDataViewColumn(
    "Name",
    new wxDataViewTextRenderer(),
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
  mProfilesCtrl->AppendColumn(name);
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Settings::onProfileSelected,
    this
  );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Settings::onProfileContext,
    this
  );

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
  hsizer->Add(mProfileOptionsSizer, 1, wxEXPAND);
  mProfiles->SetSizer(hsizer);
}

void Settings::setupProfileOptions(wxPanel *parent)
{
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

  pfp.at(Properties::Name) =
    pfg->Append(new wxStringProperty("Name", "", {}));

  mGridCategoryBroker = new wxPropertyCategory("Broker");
  mProfileGrid->Append(mGridCategoryBroker);

  pfp.at(Properties::Hostname) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Hostname", "", {}));
  pfp.at(Properties::Port) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Port", "", {}));
  pfp.at(Properties::Username) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Username", "", {}));
  pfp.at(Properties::Password) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Password", "", {}));
  pfp.at(Properties::ConnectTimeout) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Connect Timeout (s)", "", {}));
  pfp.at(Properties::DisconnectTimeout) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Disconnect Timeout (s)", "", {}));
  pfp.at(Properties::MaxInFlight) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Max in flight", "", {}));
  pfp.at(Properties::KeepAlive) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Keep alive interval", "", {}));
  pfp.at(Properties::ClientId) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Client ID", "", {}));
  pfp.at(Properties::AutoReconnect) =
    pfg->AppendIn(mGridCategoryBroker, new wxBoolProperty("Auto Reconnect", "", {}));
  pfp.at(Properties::MaxReconnectRetries) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Max Reconnect Retries", "", {}));

  mGridCategoryClient = new wxPropertyCategory("Client");
  mProfileGrid->Append(mGridCategoryClient);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfp.at(Properties::Layout) = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  mProfileGrid->Bind(wxEVT_PG_CHANGED, &Settings::onGridChanged, this);
  mProfileGrid->Bind(wxEVT_PG_CHANGING, &Settings::onGridChanged, this);
  mProfileGrid->Bind(wxEVT_PG_LABEL_EDIT_BEGIN, &Settings::onGridChanged, this);
  mProfileGrid->Bind(wxEVT_PG_SELECTED, &Settings::onGridChanged, this);

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

void Settings::onContextSelected(wxCommandEvent &event)
{
  switch (static_cast<ContextIDs>(event.GetId()))
  {
    case ContextIDs::LayoutsDelete: onLayoutsDelete(event); break;
    case ContextIDs::LayoutsRename: onLayoutsRename(event); break;
    case ContextIDs::ProfilesDelete: onProfileDelete(event); break;
  }
  event.Skip();
}

void Settings::onLayoutsDelete(wxCommandEvent &/* event */)
{
  mLogger->info("Requesting delete");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  if (mLayoutsModel->getDefault() == item) { return; }
  mLayoutsModel->remove(item);
}

void Settings::onLayoutsRename(wxCommandEvent &/* event */)
{
  mLogger->info("Requesting rename");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mLayoutsCtrl->EditItem(item, mLayoutColumnName);
}

void Settings::onLayoutsEdit(wxDataViewEvent &event)
{
  mLogger->info("Requesting edit");
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

void Settings::onProfileContext(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk())
  {
    event.Skip();
    return;
  }

  wxMenu menu;

  auto *del = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::ProfilesDelete),
    "Delete"
  );
  del->SetBitmap(mArtProvider.bitmap(Icon::Delete));
  menu.Append(del);

  PopupMenu(&menu);
}

void Settings::onProfileDelete(wxCommandEvent & /* event */)
{
  const auto item = mProfilesCtrl->GetSelection();
  mProfilesModel->remove(item);
  propertyGridClear();
}

void Settings::onProfileSelected(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    mProfileDelete->Disable();
    mProfileOptionsSizer->Hide(mProfileOptions);
    mProfileOptionsSizer->Layout();
    event.Skip();
    return;
  }

  mProfileDelete->Enable();
  mProfileOptionsSizer->Show(mProfileOptions);
  mProfileOptionsSizer->Layout();

  propertyGridFill(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );
  event.Skip();
}

void Settings::onGridChanged(wxPropertyGridEvent& event)
{
  allowSave();
  event.Skip();
}

void Settings::onButtonClickedNewProfile(wxCommandEvent &event)
{
  (void)event;
  createProfile();
}

void Settings::onButtonClickedCancel(wxCommandEvent &event)
{
  (void)event;

  propertyGridClear();

  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  propertyGridFill(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );

  allowConnect();
}

void Settings::onButtonClickedSave(wxCommandEvent &event)
{
  (void)event;

  const auto item = mProfilesCtrl->GetSelection();

  if (!item.IsOk())
  {
    return;
  }

  const auto wxs = mProfileProperties.at(Properties::Name)->GetValue().GetString();
  const auto utf8 = wxs.ToUTF8();
  const std::string name(utf8.data(), utf8.length());
  mProfilesModel->rename(item, name);

  const auto brokerOptions = brokerOptionsFromPropertyGrid();
  mProfilesModel->updateBrokerOptions(item, brokerOptions);

  const auto clientOptions = clientOptionsFromPropertyGrid();
  mProfilesModel->updateClientOptions(item, clientOptions);

  allowConnect();
}

void Settings::onButtonClickedConnect(wxCommandEvent &event)
{
  (void)event;

  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk())
  {
    return;
  }

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

void Settings::allowSave()
{
  mSave->Enable(true);
  mCancel->Enable(true);
  mProfileButtonsSizer->Show(mSave);
  mProfileButtonsSizer->Show(mCancel);

  mConnect->Enable(false);
  mProfileButtonsSizer->Hide(mConnect);

  mProfileButtonsSizer->Layout();
}

void Settings::allowConnect()
{
  mSave->Enable(false);
  mCancel->Enable(false);
  mProfileButtonsSizer->Hide(mSave);
  mProfileButtonsSizer->Hide(mCancel);

  mConnect->Enable(true);
  mProfileButtonsSizer->Show(mConnect);

  mProfileButtonsSizer->Layout();
}

void Settings::propertyGridClear()
{
  mSave->Enable(false);
  mConnect->Enable(false);
  mProfileGrid->Enable(false);

  auto &pfp = mProfileProperties;

  pfp.at(Properties::Name)->SetValue({});
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
  const wxString &name,
  const MQTT::BrokerOptions &brokerOptions,
  const Types::ClientOptions &clientOptions
) {
  auto &pfp = mProfileProperties;

  pfp.at(Properties::Name)->SetValue(name);

  pfp.at(Properties::AutoReconnect)->SetValue(brokerOptions.getAutoReconnect());
  pfp.at(Properties::ClientId)->SetValue(brokerOptions.getClientId());
  pfp.at(Properties::ConnectTimeout)->SetValue(static_cast<int>(brokerOptions.getConnectTimeout().count()));
  pfp.at(Properties::DisconnectTimeout)->SetValue(static_cast<int>(brokerOptions.getDisconnectTimeout().count()));
  pfp.at(Properties::Hostname)->SetValue(brokerOptions.getHostname());
  pfp.at(Properties::KeepAlive)->SetValue(static_cast<int>(brokerOptions.getKeepAliveInterval().count()));
  pfp.at(Properties::MaxInFlight)->SetValue(static_cast<int>(brokerOptions.getMaxInFlight()));
  pfp.at(Properties::MaxReconnectRetries)->SetValue(static_cast<int>(brokerOptions.getMaxReconnectRetries()));
  pfp.at(Properties::Password)->SetValue(brokerOptions.getPassword());
  pfp.at(Properties::Port)->SetValue(static_cast<uint16_t>(brokerOptions.getPort()));
  pfp.at(Properties::Username)->SetValue(brokerOptions.getUsername());

  (void)clientOptions;
  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();
  const bool hasValue = pfpLayout->StringToValue(layoutValue, clientOptions.getLayout());
  if (hasValue)
  {
    pfpLayout->SetValue(layoutValue);
  }

  mSave->Enable(true);
  mConnect->Enable(true);
  mProfileGrid->Enable(true);
}

MQTT::BrokerOptions Settings::brokerOptionsFromPropertyGrid() const
{
  const auto &pfp = mProfileProperties;

  const bool autoReconnect       = pfp.at(Properties::AutoReconnect)->GetValue();
  const auto maxInFlight         = static_cast<size_t>(pfp.at(Properties::MaxInFlight)->GetValue().GetInteger());
  const auto maxReconnectRetries = static_cast<size_t>(pfp.at(Properties::MaxReconnectRetries)->GetValue().GetInteger());
  const auto port                = static_cast<uint16_t>(pfp.at(Properties::Port)->GetValue().GetInteger());
  const auto connectTimeout      = static_cast<size_t>(pfp.at(Properties::ConnectTimeout)->GetValue().GetInteger());
  const auto disconnectTimeout   = static_cast<size_t>(pfp.at(Properties::DisconnectTimeout)->GetValue().GetInteger());
  const auto keepAliveInterval   = static_cast<size_t>(pfp.at(Properties::KeepAlive)->GetValue().GetLong());
  const auto clientId            = pfp.at(Properties::ClientId)->GetValue();
  const auto hostname            = pfp.at(Properties::Hostname)->GetValue();
  const auto password            = pfp.at(Properties::Password)->GetValue();
  const auto username            = pfp.at(Properties::Username)->GetValue();

  return MQTT::BrokerOptions {
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

Types::ClientOptions Settings::clientOptionsFromPropertyGrid() const
{
  const auto &pfp = mProfileProperties;

  const auto &pfpLayout = pfp.at(Properties::Layout);
  const auto layoutValue = pfpLayout->GetValue();
  const auto layoutIndex = static_cast<size_t>(layoutValue.GetInteger());
  const auto layout = mLayoutsModel->getLabelArray()[layoutIndex];

  return Types::ClientOptions {
    layout.ToStdString(),
  };
}

void Settings::onLayoutAdded(Events::Layout &/* event */)
{
  refreshLayouts();
}

void Settings::onLayoutRemoved(Events::Layout &/* event */)
{
  refreshLayouts();
}

void Settings::onLayoutSelected(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    mLayoutDelete->Disable();
    event.Skip();
    return;
  }

  if (mLayoutsModel->getDefault() == item)
  {
    mLayoutDelete->Disable();
    event.Skip();
    return;
  }

  mLayoutDelete->Enable();
  event.Skip();
}

void Settings::onLayoutChanged(Events::Layout &/* event */)
{
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

void Settings::refreshLayouts()
{
  auto &pfp = mProfileProperties;
  auto &pfg = mProfileGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  const auto layoutLabels = mLayoutsModel->getLabelArray();

  const auto selectedValue = [&]() -> std::optional<wxString>
  {
    if (!mProfileOptions->IsShown()) { return std::nullopt; }

    wxVariant layoutValue = pfpLayout->GetValue();
    const auto layoutName = pfpLayout->ValueToString(layoutValue);
    mLogger->debug("Previous layout was: {}", layoutName.ToStdString());

    return layoutName;
  }();

  pfg->RemoveProperty(pfpLayout);

  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  if (selectedValue)
  {
    wxVariant newLayoutValue;
    const bool containsValue = pfpLayout->StringToValue(newLayoutValue, selectedValue.value());
    if (containsValue)
    {
      pfpLayout->SetValue(newLayoutValue);
    }
  }
}

void Settings::onSectionSelected(wxListEvent &event)
{
  const auto selected = event.GetIndex();
  if (selected == 0)
  {
    mSectionSizer->Hide(mProfiles);
    mSectionSizer->Show(mLayouts);
    mSectionSizer->Layout();
  }
  else
  {
    mSectionSizer->Show(mProfiles);
    mSectionSizer->Hide(mLayouts);
    mSectionSizer->Layout();
  }
}
