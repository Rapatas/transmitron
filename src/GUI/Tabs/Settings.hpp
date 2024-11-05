#pragma once

#include <spdlog/spdlog.h>
#include <wx/dataview.h>
#include <wx/panel.h>
#include <wx/propgrid/property.h>
#include <wx/propgrid/props.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "GUI/ArtProvider.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Models/Profiles.hpp"

namespace Rapatas::Transmitron::GUI::Tabs {

class Settings : public wxPanel
{
public:

  explicit Settings(
    wxWindow *parent,
    const ArtProvider &artProvider,
    wxFontInfo labelFont,
    int optionsHeight,
    const wxObjectDataPtr<Models::Profiles> &profilesModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );

  void createProfile();
  void selectProfile(wxDataViewItem profile);

private:

  enum class ContextIDs : uint8_t {
    LayoutsDelete,
    LayoutsRename,
    ProfilesDelete,
    ProfilesNewFolder,
    ProfilesNewProfile,
    ProfilesRename,
  };

  enum Properties : uint8_t {
    AutoReconnect,
    ClientId,
    ConnectTimeout,
    DisconnectTimeout,
    Hostname,
    KeepAlive,
    MaxInFlight,
    MaxReconnectRetries,
    Password,
    Port,
    SSL,
    Username,
    Layout,
    Max,
  };

  wxFontInfo mLabelFont;
  int mOptionsHeight;
  const ArtProvider &mArtProvider;

  std::shared_ptr<spdlog::logger> mLogger;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxBoxSizer *mProfileOptionsSizer = nullptr;
  wxButton *mProfileDelete = nullptr;
  wxButton *mConnect = nullptr;
  wxButton *mSave = nullptr;
  wxButton *mCancel = nullptr;
  bool mProfilesWasExpanded = false;
  std::pair<bool, wxDataViewItem> mProfilesPossible;
  wxDataViewColumn *mProfileColumnName = nullptr;

  std::vector<wxPGProperty *> mProfileProperties;
  wxBoxSizer *mProfileButtonsSizer = nullptr;
  wxPanel *mProfileOptions = nullptr;
  wxPropertyCategory *mGridCategoryBroker = nullptr;
  wxPropertyCategory *mGridCategoryClient = nullptr;
  wxPropertyGrid *mProfileGrid = nullptr;

  // Layouts.
  wxPanel *mLayouts = nullptr;
  wxDataViewListCtrl *mLayoutsCtrl = nullptr;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxDataViewColumn *mLayoutColumnName = nullptr;
  wxButton *mLayoutDelete = nullptr;

  // Navigation.
  wxListCtrl *mSections = nullptr;
  wxBoxSizer *mSectionSizer = nullptr;

  void setupLayouts(wxPanel *parent);
  void setupProfiles(wxPanel *parent);
  void setupProfileOptions(wxPanel *parent);
  void setupProfileButtons(wxPanel *parent);

  void propertyGridClear();
  void propertyGridFill(
    const MQTT::BrokerOptions &brokerOptions,
    const Types::ClientOptions &clientOptions
  );
  [[nodiscard]] MQTT::BrokerOptions brokerOptionsFromPropertyGrid() const;
  [[nodiscard]] Types::ClientOptions clientOptionsFromPropertyGrid() const;
  void allowSaveProfile();
  void allowConnect();
  void refreshLayouts();

  void onContextSelected(wxCommandEvent &event);

  void onLayoutsContext(wxDataViewEvent &event);
  void onLayoutsDelete(wxCommandEvent &event);
  void onLayoutsRename(wxCommandEvent &event);
  void onLayoutsEdit(wxDataViewEvent &event);

  void onLayoutAdded(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);
  void onLayoutRemoved(Events::Layout &event);
  void onLayoutSelected(wxDataViewEvent &event);

  void onProfileContext(wxDataViewEvent &event);
  void onProfileDelete(wxCommandEvent &event);
  void onProfileRename(wxCommandEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onProfileDrag(wxDataViewEvent &event);
  void onProfileDrop(wxDataViewEvent &event);
  void onProfileDropPossible(wxDataViewEvent &event);
  void onProfileNewFolder(wxCommandEvent &event);
  void onProfileNewProfile(wxCommandEvent &event);

  void onProfileGridChanged(wxPropertyGridEvent &event);

  void onButtonClickedNewProfile(wxCommandEvent &event);
  void onButtonClickedProfileDelete(wxCommandEvent &event);
  void onButtonClickedCancel(wxCommandEvent &event);
  void onButtonClickedSave(wxCommandEvent &event);
  void onButtonClickedConnect(wxCommandEvent &event);

  void onSectionSelected(wxListEvent &event);
};

} // namespace Rapatas::Transmitron::GUI::Tabs
