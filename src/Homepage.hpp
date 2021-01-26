#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include "Events/ConnectionEvent.hpp"
#include "Models/Connections.hpp"

#include <wx/propgrid/props.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>

class Homepage :
  public wxPanel
{
public:

  explicit Homepage(wxWindow *parent = nullptr);
  virtual ~Homepage();

private:

  wxObjectDataPtr<Connections> mConnectionsModel;
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

  void fillPropertyGrid(const Connection &c);
  Connection fromPropertyGrid() const;
};

#endif // HOMEPAGE_H
