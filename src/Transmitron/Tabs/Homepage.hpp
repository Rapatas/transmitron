#ifndef TRANSMITRON_TABS_HOMEPAGE_HPP
#define TRANSMITRON_TABS_HOMEPAGE_HPP

#include <wx/propgrid/property.h>
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

  enum Properties : size_t
  {
    AutoReconnect,
    ClientId,
    ConnectTimeout,
    DisconnectTimeout,
    Hostname,
    KeepAlive,
    MaxInFlight,
    MaxReconnectRetries,
    Name,
    Password,
    Port,
    Username,
    Max
  };

  wxButton *mConnect = nullptr;
  wxButton *mSave = nullptr;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;

  // Profile Form.
  wxPanel *mProfileForm = nullptr;
  wxPropertyGrid *mProfileFormGrid = nullptr;
  std::vector<wxPGProperty*> mProfileFormProperties;

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onSaveClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onProfileDelete(wxCommandEvent &event);

  void setupProfiles();
  void setupProfileForm();
  void fillPropertyGrid(
    const MQTT::BrokerOptions &brokerOptions,
    const std::string &name
  );
  MQTT::BrokerOptions optionsFromPropertyGrid() const;
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
