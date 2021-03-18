#include "Homepage.hpp"

#include <wx/button.h>
#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>

#define wxLOG_COMPONENT "Homepage"

using namespace Transmitron::Tabs;
using namespace Transmitron;

wxDEFINE_EVENT(Events::CONNECTION, Events::Connection);

Homepage::Homepage(
  wxWindow *parent,
  wxObjectDataPtr<Models::Profiles> profilesModel
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
}

Homepage::~Homepage() {}

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

  auto newProfile = new wxButton(mProfiles, -1, "New Profile");
  newProfile->Bind(wxEVT_BUTTON, &Homepage::onNewProfileClicked, this);
  newProfile->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));

  mProfilesCtrl = new wxDataViewCtrl(mProfiles, -1);
  mProfilesCtrl->AssociateModel(mProfilesModel.get());
  mProfilesCtrl->AppendColumn(name);
  mProfilesCtrl->AppendColumn(url);
  mProfilesCtrl->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &Homepage::onProfileActivated, this);
  mProfilesCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Homepage::onProfileSelected, this);

  mConnect = new wxButton(mProfiles, -1, "Connect");
  mConnect->Enable(false);
  mConnect->Bind(wxEVT_BUTTON, &Homepage::onConnectClicked, this);
  mConnect->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK));

  auto hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(newProfile, 0, wxEXPAND);
  hsizer->AddStretchSpacer(1);
  hsizer->Add(mConnect, 0, wxEXPAND);
  auto vsizer = new wxBoxSizer(wxVERTICAL);
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
  mTimeoutProp = mProp->Append(new wxUIntProperty("Timeout (s)", "", {}));
  mMaxInFlightProp = mProp->Append(new wxUIntProperty("Max in flight", "", {}));
  mKeepAliveProp = mProp->Append(new wxUIntProperty("Keep alive interval", "", {}));
  mClientIdProp = mProp->Append(new wxStringProperty("Client ID", "", {}));
  mUsernameProp = mProp->Append(new wxStringProperty("Username", "", {}));
  mPasswordProp = mProp->Append(new wxStringProperty("Password", "", {}));
  mAutoReconnectProp = mProp->Append(new wxBoolProperty("Auto Reconnect", "", {}));

  mSave = new wxButton(mProfileForm, -1, "Save");
  mSave->Enable(false);

  auto sizer = new wxBoxSizer(wxVERTICAL);
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

  auto connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);

  e.Skip();
}

void Homepage::onProfileSelected(wxDataViewEvent &e)
{
  fillPropertyGrid(
    mProfilesModel->getBrokerOptions(e.GetItem()),
    mProfilesModel->getName(e.GetItem())
  );
  e.Skip();
}

void Homepage::onConnectClicked(wxCommandEvent &e)
{
  const auto profileItem = mProfilesCtrl->GetSelection();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);

  wxLogMessage(
    "Queueing event for profile at %s:%d",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onSaveClicked(wxCommandEvent &event)
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

void Homepage::onNewProfileClicked(wxCommandEvent &event)
{
  auto item = mProfilesModel->createProfile();
  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
  fillPropertyGrid(
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getName(item)
  );
}

void Homepage::fillPropertyGrid(
  const ValueObjects::BrokerOptions &brokerOptions,
  const std::string &name
) {
  mNameProp->SetValue(name);
  mHostnameProp->SetValue(brokerOptions.getHostname());
  mPortProp->SetValue((int)brokerOptions.getPort());
  mTimeoutProp->SetValue((int)brokerOptions.getTimeout());
  mMaxInFlightProp->SetValue((int)brokerOptions.getMaxInFlight());
  mKeepAliveProp->SetValue((int)brokerOptions.getKeepAliveInterval());
  mClientIdProp->SetValue(brokerOptions.getClientId());
  mUsernameProp->SetValue(brokerOptions.getUsername());
  mPasswordProp->SetValue(brokerOptions.getPassword());
  mAutoReconnectProp->SetValue(brokerOptions.getAutoReconnect());

  mSave->Enable(true);
  mConnect->Enable(true);
  mProp->Enable(true);
}

ValueObjects::BrokerOptions Homepage::optionsFromPropertyGrid() const
{
  return ValueObjects::BrokerOptions {
    mAutoReconnectProp->GetValue(),
    mClientIdProp->GetValue(),
    mHostnameProp->GetValue(),
    mUsernameProp->GetValue(),
    mPasswordProp->GetValue(),
    (unsigned)mKeepAliveProp->GetValue().GetLong(),
    (unsigned)mMaxInFlightProp->GetValue().GetInteger(),
    (unsigned)mPortProp->GetValue().GetInteger(),
    (unsigned)mTimeoutProp->GetValue().GetInteger(),
  };
}
