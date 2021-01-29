#ifndef TRANSMITRON_TABS_HOMEPAGE_HPP
#define TRANSMITRON_TABS_HOMEPAGE_HPP

#include <wx/propgrid/props.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>

#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Models/Connections.hpp"

namespace Transmitron::Tabs
{

class Homepage :
  public wxPanel
{
public:

  explicit Homepage(wxWindow *parent = nullptr);
  virtual ~Homepage();

private:

  wxObjectDataPtr<Models::Connections> mConnectionsModel;
  wxDataViewCtrl *mConnectionsCtrl;

  wxBoxSizer *mSizer;
  wxPanel *mConnections;
  void setupConnections();

  void onConnectionActivated(wxDataViewEvent &event);
  void onConnectionSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onSaveClicked(wxCommandEvent &event);
  void onNewConnectionClicked(wxCommandEvent &event);

  wxPanel *mConnectionForm;
  void setupConnectionForm();

  wxPropertyGrid *mProp;
  wxPGProperty *mNameProp;
  wxPGProperty *mHostnameProp;
  wxPGProperty *mPortProp;
  wxPGProperty *mTimeoutProp;
  wxPGProperty *mMaxInFlightProp;
  wxPGProperty *mKeepAliveProp;
  wxPGProperty *mClientIdProp;
  wxPGProperty *mUsernameProp;
  wxPGProperty *mPasswordProp;
  wxPGProperty *mAutoReconnectProp;

  wxButton *mSave;
  wxButton *mConnect;

  void fillPropertyGrid(const Types::Connection &c);
  Types::Connection fromPropertyGrid() const;
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
