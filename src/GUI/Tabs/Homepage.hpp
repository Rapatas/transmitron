#pragma once

#include <spdlog/spdlog.h>
#include <wx/dataview.h>
#include <wx/panel.h>
#include <wx/propgrid/property.h>
#include <wx/propgrid/props.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>

#include "GUI/ArtProvider.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Models/ProfilesWrapper.hpp"

namespace Rapatas::Transmitron::GUI::Tabs {

class Homepage : public wxPanel
{
public:

  explicit Homepage(
    wxWindow *parent,
    const ArtProvider &artProvider,
    wxFontInfo labelFont,
    int optionsHeight,
    const wxObjectDataPtr<Models::Profiles> &profilesModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );

  void focus();

private:

  enum class ContextIDs : uint8_t {
    ProfilesConnect,
    ProfilesCreate,
    ProfilesEdit,
  };

  std::shared_ptr<spdlog::logger> mLogger;
  wxFontInfo mLabelFont;
  int mOptionsHeight;
  const ArtProvider &mArtProvider;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::ProfilesWrapper> mProfilesModelWrapper;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxButton *mProfileCreate = nullptr;
  wxButton *mProfileEdit = nullptr;
  wxButton *mProfileConnect = nullptr;

  // Recordings.
  wxPanel *mRecordings = nullptr;

  // Info.
  wxPanel *mInfo = nullptr;

  // Quick Connect.
  wxPanel *mQuickConnect = nullptr;
  wxTextCtrl *mQuickConnectUrl = nullptr;
  wxButton *mQuickConnectBtn = nullptr;

  void onCancelClicked(wxCommandEvent &event);
  void onRecordingOpen(wxCommandEvent &event);

  void onConnectClicked(wxCommandEvent &event);
  void onContextSelected(wxCommandEvent &event);

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onProfileCreate(wxCommandEvent &event);
  void onProfileEdit(wxCommandEvent &event);
  void onProfileConnect(wxCommandEvent &event);
  void onProfileSelected(wxDataViewEvent &event);

  void setupProfiles(wxPanel *parent);
  void setupQuickConnect(wxPanel *parent);
  void setupRecordings(wxPanel *parent);
  void setupInfo(wxPanel *parent);

  void onQuickConnect();

  void connectTo(wxDataViewItem profile);
};

} // namespace Rapatas::Transmitron::GUI::Tabs
