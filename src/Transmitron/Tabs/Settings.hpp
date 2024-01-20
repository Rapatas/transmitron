#ifndef TRANSMITRON_TABS_SETTINGS_HPP
#define TRANSMITRON_TABS_SETTINGS_HPP

#include <spdlog/spdlog.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/dataview.h>
#include <wx/propgrid/property.h>
#include <wx/propgrid/props.h>

#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Models/Profiles.hpp"

namespace Transmitron::Tabs
{

class Settings :
  public wxPanel
{
public:

  explicit Settings(
    wxWindow *parent,
    wxFontInfo labelFont,
    int optionsHeight,
    const wxObjectDataPtr<Models::Profiles> &profilesModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );

private:

  enum class ContextIDs : unsigned
  {
    LayoutsDelete,
    LayoutsRename,
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

  wxFontInfo mLabelFont;
  int mOptionsHeight;

  std::shared_ptr<spdlog::logger> mLogger;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxPanel *mProfilesLeft = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;

  // Profile Form.
  wxPanel *mProfileForm = nullptr;
  wxPropertyGrid *mProfileFormGrid = nullptr;
  wxPropertyCategory *mGridCategoryBroker = nullptr;
  wxPropertyCategory *mGridCategoryClient = nullptr;
  std::vector<wxPGProperty*> mProfileFormProperties;

  // Layouts.
  wxPanel *mLayouts = nullptr;
  wxDataViewListCtrl *mLayoutsCtrl = nullptr;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxDataViewColumn *mLayoutColumnName = nullptr;

  // Buttons.
  wxPanel *mProfileButtons = nullptr;
  wxBoxSizer *mProfileButtonsSizer = nullptr;
  wxButton *mConnect = nullptr;
  wxButton *mSave = nullptr;
  wxButton *mCancel = nullptr;

  void setupLayouts(wxPanel *parent);
  void setupProfiles(wxPanel *parent);
  void setupProfileForm(wxPanel *parent);
  void setupProfileButtons(wxPanel *parent);

  void propertyGridClear();
  void propertyGridFill(
    const wxString &name,
    const MQTT::BrokerOptions &brokerOptions,
    const Types::ClientOptions &clientOptions
  );
  MQTT::BrokerOptions brokerOptionsFromPropertyGrid() const;
  Types::ClientOptions clientOptionsFromPropertyGrid() const;
  void allowSave();
  void allowConnect();
  void refreshLayouts();

  void onLayoutsContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onLayoutsDelete(wxCommandEvent &event);
  void onLayoutsRename(wxCommandEvent &event);
  void onLayoutsEdit(wxDataViewEvent &event);

  void onLayoutAdded(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);
  void onLayoutRemoved(Events::Layout &event);

  void onProfileContext(wxDataViewEvent &event);
  void onProfileDelete(wxCommandEvent &event);
  void onProfileSelected(wxDataViewEvent &event);

  void onGridChanged(wxPropertyGridEvent& event);

  void onButtonClickedNewProfile(wxCommandEvent &event);
  void onButtonClickedCancel(wxCommandEvent &event);
  void onButtonClickedSave(wxCommandEvent &event);
  void onButtonClickedConnect(wxCommandEvent &event);

};

}

#endif // TRANSMITRON_TABS_SETTINGS_HPP
