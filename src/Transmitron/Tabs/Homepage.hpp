#ifndef TRANSMITRON_TABS_HOMEPAGE_HPP
#define TRANSMITRON_TABS_HOMEPAGE_HPP

#include <wx/propgrid/props.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/dataview.h>

#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Models/Profiles.hpp"

namespace Transmitron::Tabs
{

class Homepage :
  public wxPanel
{
public:

  explicit Homepage(
    wxWindow *parent,
    const wxObjectDataPtr<Models::Profiles> &profilesModel
  );

private:

  enum class ContextIDs : unsigned
  {
    ProfilesDelete,
  };

  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;

  wxBoxSizer *mSizer = nullptr;
  wxPanel *mProfiles = nullptr;
  void setupProfiles();

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onSaveClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onProfileDelete(wxCommandEvent &event);

  wxPanel *mProfileForm = nullptr;
  void setupProfileForm();

  wxPropertyGrid *mProp = nullptr;
  wxPGProperty *mAutoReconnectProp = nullptr;
  wxPGProperty *mClientIdProp = nullptr;
  wxPGProperty *mConnectTimeoutProp = nullptr;
  wxPGProperty *mDisconnectTimeoutProp = nullptr;
  wxPGProperty *mHostnameProp = nullptr;
  wxPGProperty *mKeepAliveProp = nullptr;
  wxPGProperty *mMaxInFlightProp = nullptr;
  wxPGProperty *mMaxReconnectRetriesProp = nullptr;
  wxPGProperty *mNameProp = nullptr;
  wxPGProperty *mPasswordProp = nullptr;
  wxPGProperty *mPortProp = nullptr;
  wxPGProperty *mUsernameProp = nullptr;

  wxButton *mSave = nullptr;
  wxButton *mConnect = nullptr;

  void fillPropertyGrid(
    const MQTT::BrokerOptions &brokerOptions,
    const std::string &name
  );
  MQTT::BrokerOptions optionsFromPropertyGrid() const;
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
