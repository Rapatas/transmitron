#ifndef TRANSMITRON_TABS_HOMEPAGE_HPP
#define TRANSMITRON_TABS_HOMEPAGE_HPP

#include <spdlog/spdlog.h>
#include <wx/propgrid/property.h>
#include <wx/propgrid/props.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/dataview.h>

#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Events/Layout.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Models/Profiles.hpp"
#include "Transmitron/Types/ClientOptions.hpp"

namespace Transmitron::Tabs
{

class Homepage :
  public wxPanel
{
public:

  explicit Homepage(
    wxWindow *parent,
    wxFontInfo labelFont,
    const wxObjectDataPtr<Models::Profiles> &profilesModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );

private:

  static constexpr size_t OptionsHeight = 26;

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
    Layout,
    Max,
  };

  const wxFontInfo mLabelFont;

  std::shared_ptr<spdlog::logger> mLogger;
  wxButton *mConnect = nullptr;
  wxButton *mSave = nullptr;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  // Profile Form.
  wxPanel *mProfileForm = nullptr;
  wxPropertyGrid *mProfileFormGrid = nullptr;
  wxPropertyCategory *mGridCategoryBroker = nullptr;
  wxPropertyCategory *mGridCategoryClient = nullptr;
  std::vector<wxPGProperty*> mProfileFormProperties;

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onSaveClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onProfileDelete(wxCommandEvent &event);
  void onLayoutAdded(Events::Layout &event);
  void onLayoutRemoved(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);

  void setupProfiles();
  void setupProfileForm();
  void fillPropertyGrid(
    const wxString &name,
    const MQTT::BrokerOptions &brokerOptions,
    const Types::ClientOptions &clientOptions
  );
  MQTT::BrokerOptions brokerOptionsFromPropertyGrid() const;
  Types::ClientOptions clientOptionsFromPropertyGrid() const;
  void refreshLayouts();
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
