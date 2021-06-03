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
  wxFontInfo labelFont,
  const wxObjectDataPtr<Models::Profiles> &profilesModel
) :
  wxPanel(parent),
  mLabelFont(std::move(labelFont)),
  mProfilesModel(profilesModel)
{
  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  this->SetSizer(hsizer);

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
  pfp.at(Properties::Hostname) =
    pfg->Append(new wxStringProperty("Hostname", "", {}));
  pfp.at(Properties::Port) =
    pfg->Append(new wxUIntProperty("Port", "", {}));
  pfp.at(Properties::Username) =
    pfg->Append(new wxStringProperty("Username", "", {}));
  pfp.at(Properties::Password) =
    pfg->Append(new wxStringProperty("Password", "", {}));
  pfp.at(Properties::ConnectTimeout) =
    pfg->Append(new wxUIntProperty("Connect Timeout (s)", "", {}));
  pfp.at(Properties::DisconnectTimeout) =
    pfg->Append(new wxUIntProperty("Disconnect Timeout (s)", "", {}));
  pfp.at(Properties::MaxInFlight) =
    pfg->Append(new wxUIntProperty("Max in flight", "", {}));
  pfp.at(Properties::KeepAlive) =
    pfg->Append(new wxUIntProperty("Keep alive interval", "", {}));
  pfp.at(Properties::ClientId) =
    pfg->Append(new wxStringProperty("Client ID", "", {}));
  pfp.at(Properties::AutoReconnect) =
    pfg->Append(new wxBoolProperty("Auto Reconnect", "", {}));
  pfp.at(Properties::MaxReconnectRetries) =
    pfg->Append(new wxUIntProperty("Max Reconnect Retries", "", {}));

  mSave = new wxButton(mProfileForm, -1, "Save");
  mSave->Enable(false);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(label,            0, wxEXPAND);
  sizer->Add(mProfileFormGrid, 1, wxEXPAND);
  sizer->Add(mSave,            0, wxEXPAND);
  mProfileForm->SetSizer(sizer);

  mSave->Bind(wxEVT_BUTTON, &Homepage::onSaveClicked, this);
}

void Homepage::onProfileActivated(wxDataViewEvent &e)
{
  const auto &profileItem = e.GetItem();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);
  wxLogMessage(
    "Queueing event for profile at %s:%zu",
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
    "Queueing event for profile at %s:%zu",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onSaveClicked(wxCommandEvent &/* event */)
{
  const auto item = mProfilesCtrl->GetSelection();

  if (!item.IsOk())
  {
    return;
  }

  const auto name = mProfileFormProperties.at(Properties::Name)->GetValue();
  mProfilesModel->updateName(item, name);

  const auto options = optionsFromPropertyGrid();
  mProfilesModel->updateBrokerOptions(item, options);
}

void Homepage::onNewProfileClicked(wxCommandEvent &/* event */)
{
  const auto item = mProfilesModel->createProfile();
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
  auto &pfp = mProfileFormProperties;
  pfp.at(Properties::AutoReconnect)->SetValue(brokerOptions.getAutoReconnect());
  pfp.at(Properties::ClientId)->SetValue(brokerOptions.getClientId());
  pfp.at(Properties::ConnectTimeout)->SetValue((int)brokerOptions.getConnectTimeout().count());
  pfp.at(Properties::DisconnectTimeout)->SetValue(brokerOptions.getDisconnectTimeout().count());
  pfp.at(Properties::Hostname)->SetValue(brokerOptions.getHostname());
  pfp.at(Properties::KeepAlive)->SetValue(brokerOptions.getKeepAliveInterval().count());
  pfp.at(Properties::MaxInFlight)->SetValue((int)brokerOptions.getMaxInFlight());
  pfp.at(Properties::MaxReconnectRetries)->SetValue((int)brokerOptions.getMaxReconnectRetries());
  pfp.at(Properties::Name)->SetValue(name);
  pfp.at(Properties::Password)->SetValue(brokerOptions.getPassword());
  pfp.at(Properties::Port)->SetValue((int)brokerOptions.getPort());
  pfp.at(Properties::Username)->SetValue(brokerOptions.getUsername());

  mSave->Enable(true);
  mConnect->Enable(true);
  mProfileFormGrid->Enable(true);
}

MQTT::BrokerOptions Homepage::optionsFromPropertyGrid() const
{
  const auto &pfp = mProfileFormProperties;
  return MQTT::BrokerOptions {
    pfp.at(Properties::AutoReconnect)->GetValue(),
    (unsigned)pfp.at(Properties::MaxInFlight)->GetValue().GetInteger(),
    (unsigned)pfp.at(Properties::MaxReconnectRetries)->GetValue().GetInteger(),
    (unsigned)pfp.at(Properties::Port)->GetValue().GetInteger(),
    (unsigned)pfp.at(Properties::ConnectTimeout)->GetValue().GetInteger(),
    (unsigned)pfp.at(Properties::DisconnectTimeout)->GetValue().GetInteger(),
    (unsigned)pfp.at(Properties::KeepAlive)->GetValue().GetLong(),
    pfp.at(Properties::ClientId)->GetValue(),
    pfp.at(Properties::Hostname)->GetValue(),
    pfp.at(Properties::Password)->GetValue(),
    pfp.at(Properties::Username)->GetValue(),
  };
}
