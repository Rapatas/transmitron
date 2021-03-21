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
  virtual ~Homepage();

private:

  enum class ContextIDs : unsigned
  {
    ProfilesDelete,
  };

  wxDataViewCtrl *mProfilesCtrl;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;

  wxBoxSizer *mSizer;
  wxPanel *mProfiles;
  void setupProfiles();

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onSaveClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onProfileDelete(wxCommandEvent &event);

  wxPanel *mProfileForm;
  void setupProfileForm();

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

  void fillPropertyGrid(
    const ValueObjects::BrokerOptions &brokerOptions,
    const std::string &name
  );
  ValueObjects::BrokerOptions optionsFromPropertyGrid() const;
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
