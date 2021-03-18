#include "Homepage.hpp"

#include <wx/button.h>
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>

#define wxLOG_COMPONENT "Homepage"

using namespace Transmitron::Tabs;
using namespace Transmitron;

wxDEFINE_EVENT(Events::CONNECTION, Events::Connection);

Homepage::Homepage(
  wxWindow *parent,
  wxObjectDataPtr<Models::Connections> connectionsModel
) :
  wxPanel(parent),
  mConnectionsModel(connectionsModel)
{
  mSizer = new wxBoxSizer(wxHORIZONTAL);
  this->SetSizer(mSizer);

  setupConnections();
  setupConnectionForm();

  mSizer->Add(mConnections,      1, wxEXPAND);
  mSizer->Add(mConnectionForm,   1, wxEXPAND);

  mSizer->Layout();
}

Homepage::~Homepage() {}

void Homepage::setupConnections()
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    "name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Connections::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const url = new wxDataViewColumn(
    "url",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Connections::Column::URL,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mConnections = new wxPanel(this, -1);

  auto newConnection = new wxButton(mConnections, -1, "New Connection");

  mConnectionsCtrl = new wxDataViewCtrl(mConnections, -1);
  mConnectionsCtrl->AssociateModel(mConnectionsModel.get());
  mConnectionsCtrl->AppendColumn(name);
  mConnectionsCtrl->AppendColumn(url);

  auto vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(mConnectionsCtrl, 1, wxEXPAND);
  vsizer->Add(newConnection,    0, wxEXPAND);
  mConnections->SetSizer(vsizer);

  mConnectionsCtrl->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &Homepage::onConnectionActivated, this);
  mConnectionsCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Homepage::onConnectionSelected, this);
  newConnection->Bind(wxEVT_BUTTON, &Homepage::onNewConnectionClicked, this);
}

void Homepage::setupConnectionForm()
{
  mConnectionForm = new wxPanel(this, -1);

  mProp = new wxPropertyGrid(
    mConnectionForm,
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

  mSave    = new wxButton(mConnectionForm, -1, "Save");
  mConnect = new wxButton(mConnectionForm, -1, "Connect");

  mSave->Enable(false);
  mConnect->Enable(false);

  auto sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(mProp, 1, wxEXPAND);
  sizer->Add(mConnect, 0, wxEXPAND);
  sizer->Add(mSave, 0, wxEXPAND);
  mConnectionForm->SetSizer(sizer);

  mConnect->Bind(wxEVT_BUTTON, &Homepage::onConnectClicked, this);
  mSave->Bind(wxEVT_BUTTON, &Homepage::onSaveClicked, this);
}

void Homepage::onConnectionActivated(wxDataViewEvent &e)
{
  const auto &connectionItem = e.GetItem();
  const auto &brokerOptions = mConnectionsModel->getBrokerOptions(connectionItem);
  wxLogMessage(
    "Queueing event for connection at %s:%d",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto connectionEvent = new Events::Connection();
  connectionEvent->setConnection(connectionItem);
  wxQueueEvent(this, connectionEvent);

  e.Skip();
}

void Homepage::onConnectionSelected(wxDataViewEvent &e)
{
  fillPropertyGrid(
    mConnectionsModel->getBrokerOptions(e.GetItem()),
    mConnectionsModel->getName(e.GetItem())
  );
  e.Skip();
}

void Homepage::onConnectClicked(wxCommandEvent &e)
{
  const auto connectionItem = mConnectionsCtrl->GetSelection();
  const auto &brokerOptions = mConnectionsModel->getBrokerOptions(connectionItem);

  wxLogMessage(
    "Queueing event for connection at %s:%d",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto connectionEvent = new Events::Connection();
  connectionEvent->setConnection(connectionItem);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onSaveClicked(wxCommandEvent &event)
{
  auto item = mConnectionsCtrl->GetSelection();

  if (!item.IsOk())
  {
    return;
  }

  auto options = optionsFromPropertyGrid();
  auto name = mNameProp->GetValue();

  mConnectionsModel->updateBrokerOptions(item, options);
  mConnectionsModel->updateName(item, name);
}

void Homepage::onNewConnectionClicked(wxCommandEvent &event)
{
  auto item = mConnectionsModel->createConnection();
  mConnectionsCtrl->Select(item);
  mConnectionsCtrl->EnsureVisible(item);
  fillPropertyGrid(
    mConnectionsModel->getBrokerOptions(item),
    mConnectionsModel->getName(item)
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
