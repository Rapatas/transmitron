#include <chrono>
#include <wx/button.h>
#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>

#include "Homepage.hpp"
#include "Common/Log.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Types/ClientOptions.hpp"
#include "Transmitron/Notifiers/Layouts.hpp"

using namespace Transmitron::Tabs;
using namespace Transmitron;

wxDEFINE_EVENT(Events::CONNECTION, Events::Connection); // NOLINT

Homepage::Homepage(
  wxWindow *parent,
  wxFontInfo labelFont,
  const wxObjectDataPtr<Models::Profiles> &profilesModel,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel
) :
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    "Homepage"
  ),
  mLabelFont(std::move(labelFont)),
  mProfilesModel(profilesModel),
  mLayoutsModel(layoutsModel)
{
  mLogger = Common::Log::create("Transmitron::Homepage");

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  this->SetSizer(hsizer);

  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_ADDED,   &Homepage::onLayoutAdded,   this);
  notifier->Bind(Events::LAYOUT_REMOVED, &Homepage::onLayoutRemoved, this);
  notifier->Bind(Events::LAYOUT_CHANGED, &Homepage::onLayoutChanged, this);

  setupProfiles();
  setupProfileForm();

  hsizer->Add(mProfiles,    1, wxEXPAND);
  hsizer->Add(mProfileForm, 1, wxEXPAND);
  hsizer->Layout();

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Homepage::onContextSelected, this);
}

void Homepage::setupProfiles()
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    "Name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const url = new wxDataViewColumn(
    "Address",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::URL,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mProfiles = new wxPanel(this, -1);

  auto *label = new wxStaticText(mProfiles, -1, "Profiles");
  label->SetFont(mLabelFont);

  auto *newProfile = new wxButton(
    mProfiles,
    -1,
    "New Profile",
    wxDefaultPosition,
    wxSize(-1, OptionsHeight)
  );
  newProfile->Bind(wxEVT_BUTTON, &Homepage::onNewProfileClicked, this);
  newProfile->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));

  mProfilesCtrl = new wxDataViewCtrl(mProfiles, -1);
  mProfilesCtrl->AssociateModel(mProfilesModel.get());
  mProfilesCtrl->AppendColumn(name);
  mProfilesCtrl->AppendColumn(url);
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_ACTIVATED,
    &Homepage::onProfileActivated,
    this
   );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Homepage::onProfileSelected,
    this
   );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Homepage::onProfileContext,
    this
  );

  mConnect = new wxButton(
    mProfiles,
    -1,
    "Connect",
    wxDefaultPosition,
    wxSize(-1, OptionsHeight)
  );
  mConnect->Enable(false);
  mConnect->Bind(wxEVT_BUTTON, &Homepage::onConnectClicked, this);
  mConnect->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK));

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->SetMinSize(0, OptionsHeight);
  hsizer->Add(newProfile, 0, wxEXPAND);
  hsizer->AddStretchSpacer(1);
  hsizer->Add(mConnect, 0, wxEXPAND);
  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,         0, wxEXPAND);
  vsizer->Add(mProfilesCtrl, 1, wxEXPAND);
  vsizer->Add(hsizer,        0, wxEXPAND);
  mProfiles->SetSizer(vsizer);
}

void Homepage::setupProfileForm()
{
  mProfileForm = new wxPanel(this, -1);

  auto *label = new wxStaticText(mProfileForm, -1, "Profile Options");
  label->SetFont(mLabelFont);

  mProfileFormGrid = new wxPropertyGrid(
    mProfileForm,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxPG_SPLITTER_AUTO_CENTER
  );

  mProfileFormGrid->DedicateKey(WXK_UP);
  mProfileFormGrid->DedicateKey(WXK_DOWN);

  mProfileFormGrid->Enable(false);

  auto &pfp = mProfileFormProperties;
  auto &pfg = mProfileFormGrid;

  pfp.resize(Properties::Max);

  pfp.at(Properties::Name) =
    pfg->Append(new wxStringProperty("Name", "", {}));

  mGridCategoryBroker = new wxPropertyCategory("Broker");
  mProfileFormGrid->Append(mGridCategoryBroker);

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
  mProfileFormGrid->Append(mGridCategoryClient);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfp.at(Properties::Layout) = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  mSave = new wxButton(
    mProfileForm,
    -1,
    "Save",
    wxDefaultPosition,
    wxSize(-1, OptionsHeight)
  );
  mSave->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
  mSave->Enable(false);

  auto *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
  bottomSizer->Add(mSave, 1, wxEXPAND);
  bottomSizer->SetMinSize(0, OptionsHeight);
  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(label,            0, wxEXPAND);
  sizer->Add(mProfileFormGrid, 1, wxEXPAND);
  sizer->Add(bottomSizer,      0, wxEXPAND);
  mProfileForm->SetSizer(sizer);

  mSave->Bind(wxEVT_BUTTON, &Homepage::onSaveClicked, this);
}

void Homepage::onProfileActivated(wxDataViewEvent &e)
{
  const auto &profileItem = e.GetItem();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);
  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);

  e.Skip();
}

void Homepage::onProfileSelected(wxDataViewEvent &e)
{
  const auto item = e.GetItem();
  if (!item.IsOk())
  {
    e.Skip();
    return;
  }

  fillPropertyGrid(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );
  e.Skip();
}

void Homepage::onConnectClicked(wxCommandEvent &/* event */)
{
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

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(item);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onSaveClicked(wxCommandEvent &/* event */)
{
  const auto item = mProfilesCtrl->GetSelection();

  if (!item.IsOk())
  {
    return;
  }

  const auto wxs = mProfileFormProperties.at(Properties::Name)->GetValue().GetString();
  const auto utf8 = wxs.ToUTF8();
  const std::string name(utf8.data(), utf8.length());
  mProfilesModel->rename(item, name);

  const auto brokerOptions = brokerOptionsFromPropertyGrid();
  mProfilesModel->updateBrokerOptions(item, brokerOptions);

  const auto clientOptions = clientOptionsFromPropertyGrid();
  mProfilesModel->updateClientOptions(item, clientOptions);
}

void Homepage::onNewProfileClicked(wxCommandEvent &/* event */)
{
  const auto item = mProfilesModel->createProfile();
  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
  fillPropertyGrid(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );
}

void Homepage::onProfileContext(wxDataViewEvent &e)
{
  if (!e.GetItem().IsOk())
  {
    e.Skip();
    return;
  }

  wxMenu menu;

  auto *del = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::ProfilesDelete,
    "Delete"
  );
  del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(del);

  PopupMenu(&menu);
}

void Homepage::onContextSelected(wxCommandEvent &e)
{
  switch ((ContextIDs)e.GetId())
  {
    case ContextIDs::ProfilesDelete: onProfileDelete(e); break;
  }
  e.Skip();
}

void Homepage::onProfileDelete(wxCommandEvent & /* event */)
{
  mLogger->info("Requesting delete");
  const auto item = mProfilesCtrl->GetSelection();
  mProfilesModel->remove(item);
}

void Homepage::onLayoutAdded(Events::Layout &/* event */)
{
  refreshLayouts();
}

void Homepage::onLayoutRemoved(Events::Layout &/* event */)
{
  refreshLayouts();
}

void Homepage::onLayoutChanged(Events::Layout &/* event */)
{
  auto &pfp = mProfileFormProperties;
  auto &pfg = mProfileFormGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();

  pfg->RemoveProperty(pfpLayout);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  pfpLayout->SetValue(layoutValue);
}

void Homepage::refreshLayouts()
{
  auto &pfp = mProfileFormProperties;
  auto &pfg = mProfileFormGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();
  const auto layoutName = pfpLayout->ValueToString(layoutValue);
  mLogger->info("Previous layout was: {}", layoutName);

  pfg->RemoveProperty(pfpLayout);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  wxVariant newLayoutValue = pfpLayout->GetValue();
  const bool hasValue = pfpLayout->StringToValue(newLayoutValue, layoutName);
  if (!hasValue)
  {
    mLogger->info("Previous layout not found");
    wxVariant layoutValue = pfpLayout->GetValue();
    const wxString layoutName = std::string(Models::Layouts::DefaultName);
    pfpLayout->StringToValue(layoutValue, layoutName);
    pfpLayout->SetValue(layoutValue);
    return;
  }

  mLogger->info("Previous layout was found ");
  pfpLayout->SetValue(newLayoutValue);
}

void Homepage::fillPropertyGrid(
  const wxString &name,
  const MQTT::BrokerOptions &brokerOptions,
  const Types::ClientOptions &clientOptions
) {
  auto &pfp = mProfileFormProperties;

  pfp.at(Properties::Name)->SetValue(name);

  pfp.at(Properties::AutoReconnect)->SetValue(brokerOptions.getAutoReconnect());
  pfp.at(Properties::ClientId)->SetValue(brokerOptions.getClientId());
  pfp.at(Properties::ConnectTimeout)->SetValue((int)brokerOptions.getConnectTimeout().count());
  pfp.at(Properties::DisconnectTimeout)->SetValue((int)brokerOptions.getDisconnectTimeout().count());
  pfp.at(Properties::Hostname)->SetValue(brokerOptions.getHostname());
  pfp.at(Properties::KeepAlive)->SetValue((int)brokerOptions.getKeepAliveInterval().count());
  pfp.at(Properties::MaxInFlight)->SetValue((int)brokerOptions.getMaxInFlight());
  pfp.at(Properties::MaxReconnectRetries)->SetValue((int)brokerOptions.getMaxReconnectRetries());
  pfp.at(Properties::Password)->SetValue(brokerOptions.getPassword());
  pfp.at(Properties::Port)->SetValue((int)brokerOptions.getPort());
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
  mProfileFormGrid->Enable(true);
}

MQTT::BrokerOptions Homepage::brokerOptionsFromPropertyGrid() const
{
  const auto &pfp = mProfileFormProperties;

  const bool autoReconnect       = pfp.at(Properties::AutoReconnect)->GetValue();
  const auto maxInFlight         = (size_t)pfp.at(Properties::MaxInFlight)->GetValue().GetInteger();
  const auto maxReconnectRetries = (size_t)pfp.at(Properties::MaxReconnectRetries)->GetValue().GetInteger();
  const auto port                = (size_t)pfp.at(Properties::Port)->GetValue().GetInteger();
  const auto connectTimeout      = (size_t)pfp.at(Properties::ConnectTimeout)->GetValue().GetInteger();
  const auto disconnectTimeout   = (size_t)pfp.at(Properties::DisconnectTimeout)->GetValue().GetInteger();
  const auto keepAliveInterval   = (size_t)pfp.at(Properties::KeepAlive)->GetValue().GetLong();
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

Types::ClientOptions Homepage::clientOptionsFromPropertyGrid() const
{
  const auto &pfp = mProfileFormProperties;

  auto &pfpLayout = pfp.at(Properties::Layout);
  const auto layoutValue = pfpLayout->GetValue();
  const auto layoutIndex = (size_t)layoutValue.GetInteger();
  const auto layout = mLayoutsModel->getLabelArray()[layoutIndex];

  return Types::ClientOptions {
    layout.ToStdString(),
  };
}
