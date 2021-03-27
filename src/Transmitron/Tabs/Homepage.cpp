#include "Homepage.hpp"

#include <wx/button.h>
#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>

#define wxLOG_COMPONENT "Homepage" // NOLINT

using namespace Transmitron::Tabs;
using namespace Transmitron;

wxDEFINE_EVENT(Events::CONNECTION, Events::Connection); // NOLINT

Homepage::Homepage(
  wxWindow *parent,
  const wxObjectDataPtr<Models::Profiles> &profilesModel
) :
  wxPanel(parent),
  mProfilesModel(profilesModel)
{
  mSizer = new wxBoxSizer(wxHORIZONTAL);
  this->SetSizer(mSizer);

  setupProfiles();
  setupProfileForm();

  mSizer->Add(mProfiles,      1, wxEXPAND);
  mSizer->Add(mProfileForm,   1, wxEXPAND);

  mSizer->Layout();

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Homepage::onContextSelected, this);
}

void Homepage::setupProfiles()
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    "name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const url = new wxDataViewColumn(
    "url",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::URL,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mProfiles = new wxPanel(this, -1);

  auto *newProfile = new wxButton(mProfiles, -1, "New Profile");
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

  mConnect = new wxButton(mProfiles, -1, "Connect");
  mConnect->Enable(false);
  mConnect->Bind(wxEVT_BUTTON, &Homepage::onConnectClicked, this);
  mConnect->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK));

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(newProfile, 0, wxEXPAND);
  hsizer->AddStretchSpacer(1);
  hsizer->Add(mConnect, 0, wxEXPAND);
  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(mProfilesCtrl, 1, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  mProfiles->SetSizer(vsizer);
}

void Homepage::setupProfileForm()
{
  mProfileForm = new wxPanel(this, -1);

  mProp = new wxPropertyGrid(
    mProfileForm,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxPG_SPLITTER_AUTO_CENTER
  );

  mProp->DedicateKey(WXK_UP);
  mProp->DedicateKey(WXK_DOWN);

  mProp->Enable(false);

  mNameProp = mProp->Append(new wxStringProperty("Name", "", {}));
  mHostnameProp = mProp->Append(new wxStringProperty("Hostname", "", {}));
  mPortProp = mProp->Append(new wxUIntProperty("Port", "", {}));
  mConnectTimeoutProp = mProp->Append(new wxUIntProperty("Connect Timeout (s)", "", {}));
  mDisconnectTimeoutProp = mProp->Append(new wxUIntProperty("Disconnect Timeout (s)", "", {}));
  mMaxInFlightProp = mProp->Append(new wxUIntProperty("Max in flight", "", {}));
  mKeepAliveProp = mProp->Append(new wxUIntProperty("Keep alive interval", "", {}));
  mClientIdProp = mProp->Append(new wxStringProperty("Client ID", "", {}));
  mUsernameProp = mProp->Append(new wxStringProperty("Username", "", {}));
  mPasswordProp = mProp->Append(new wxStringProperty("Password", "", {}));
  mAutoReconnectProp = mProp->Append(new wxBoolProperty("Auto Reconnect", "", {}));
  mMaxReconnectRetriesProp = mProp->Append(new wxUIntProperty("Max Reconnect Retries", "", {}));

  mSave = new wxButton(mProfileForm, -1, "Save");
  mSave->Enable(false);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(mProp, 1, wxEXPAND);
  sizer->Add(mSave, 0, wxEXPAND);
  mProfileForm->SetSizer(sizer);

  mSave->Bind(wxEVT_BUTTON, &Homepage::onSaveClicked, this);
}

void Homepage::onProfileActivated(wxDataViewEvent &e)
{
  const auto &profileItem = e.GetItem();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);
  wxLogMessage(
    "Queueing event for profile at %s:%d",
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
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getName(item)
  );
  e.Skip();
}

void Homepage::onConnectClicked(wxCommandEvent &/* event */)
{
  const auto profileItem = mProfilesCtrl->GetSelection();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);

  wxLogMessage(
    "Queueing event for profile at %s:%d",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onSaveClicked(wxCommandEvent &/* event */)
{
  auto item = mProfilesCtrl->GetSelection();

  if (!item.IsOk())
  {
    return;
  }

  auto name = mNameProp->GetValue();
  mProfilesModel->updateName(item, name);

  auto options = optionsFromPropertyGrid();
  mProfilesModel->updateBrokerOptions(item, options);
}

void Homepage::onNewProfileClicked(wxCommandEvent &/* event */)
{
  auto item = mProfilesModel->createProfile();
  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
  fillPropertyGrid(
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getName(item)
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
  wxLogMessage("Requesting delete");
  const auto item = mProfilesCtrl->GetSelection();
  mProfilesModel->remove(item);
}

void Homepage::fillPropertyGrid(
  const MQTT::BrokerOptions &brokerOptions,
  const std::string &name
) {
  mAutoReconnectProp->SetValue(brokerOptions.getAutoReconnect());
  mClientIdProp->SetValue(brokerOptions.getClientId());
  mConnectTimeoutProp->SetValue((int)brokerOptions.getConnectTimeout().count());
  mDisconnectTimeoutProp->SetValue(brokerOptions.getDisconnectTimeout().count());
  mHostnameProp->SetValue(brokerOptions.getHostname());
  mKeepAliveProp->SetValue(brokerOptions.getKeepAliveInterval().count());
  mMaxInFlightProp->SetValue((int)brokerOptions.getMaxInFlight());
  mMaxReconnectRetriesProp->SetValue((int)brokerOptions.getMaxReconnectRetries());
  mNameProp->SetValue(name);
  mPasswordProp->SetValue(brokerOptions.getPassword());
  mPortProp->SetValue((int)brokerOptions.getPort());
  mUsernameProp->SetValue(brokerOptions.getUsername());

  mSave->Enable(true);
  mConnect->Enable(true);
  mProp->Enable(true);
}

MQTT::BrokerOptions Homepage::optionsFromPropertyGrid() const
{
  return MQTT::BrokerOptions {
    mAutoReconnectProp->GetValue(),
    (unsigned)mMaxInFlightProp->GetValue().GetInteger(),
    (unsigned)mMaxReconnectRetriesProp->GetValue().GetInteger(),
    (unsigned)mPortProp->GetValue().GetInteger(),
    (unsigned)mConnectTimeoutProp->GetValue().GetInteger(),
    (unsigned)mDisconnectTimeoutProp->GetValue().GetInteger(),
    (unsigned)mKeepAliveProp->GetValue().GetLong(),
    mClientIdProp->GetValue(),
    mHostnameProp->GetValue(),
    mPasswordProp->GetValue(),
    mUsernameProp->GetValue(),
  };
}
