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
    int optionsHeight,
    const wxObjectDataPtr<Models::Profiles> &profilesModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );

  void focus();

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
    Layout,
    Max,
  };

  const wxFontInfo mLabelFont;
  const int mOptionsHeight;

  std::shared_ptr<spdlog::logger> mLogger;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  // Recordings.
  wxPanel *mRecordings = nullptr;

  // Profile Form.
  wxPanel *mProfileForm = nullptr;
  wxPropertyGrid *mProfileFormGrid = nullptr;
  wxPropertyCategory *mGridCategoryBroker = nullptr;
  wxPropertyCategory *mGridCategoryClient = nullptr;
  std::vector<wxPGProperty*> mProfileFormProperties;

  // Buttons.
  wxPanel *mProfileButtons;
  wxBoxSizer *mProfileButtonsSizer;
  wxButton *mConnect = nullptr;
  wxButton *mSave = nullptr;
  wxButton *mCancel = nullptr;

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onSaveClicked(wxCommandEvent &event);
  void onCancelClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onProfileDelete(wxCommandEvent &event);
  void onLayoutAdded(Events::Layout &event);
  void onLayoutRemoved(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);
  void onRecordingOpen(wxCommandEvent &event);
  void onGridChanged(wxPropertyGridEvent& event);

  void setupProfiles(wxPanel *parent);
  void setupRecordings(wxPanel *parent);
  void setupProfileForm(wxPanel *parent);
  void setupProfileButtons(wxPanel *parent);
  void propertyGridFill(
    const wxString &name,
    const MQTT::BrokerOptions &brokerOptions,
    const Types::ClientOptions &clientOptions
  );
  void propertyGridClear();
  MQTT::BrokerOptions brokerOptionsFromPropertyGrid() const;
  Types::ClientOptions clientOptionsFromPropertyGrid() const;
  void refreshLayouts();
  void allowSave();
  void allowConnect();
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
