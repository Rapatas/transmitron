#pragma once

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
    ProfilesConnect,
    ProfilesCreate,
    ProfilesEdit,
  };

  wxFontInfo mLabelFont;
  int mOptionsHeight;

  std::shared_ptr<spdlog::logger> mLogger;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxButton *mProfileCreate = nullptr;
  wxButton *mProfileEdit = nullptr;
  wxButton *mProfileConnect = nullptr;

  // Recordings.
  wxPanel *mRecordings = nullptr;

  // Quick Connect.
  wxPanel *mQuickConnect = nullptr;
  wxTextCtrl *mQuickConnectUrl = nullptr;
  wxButton *mQuickConnectBtn = nullptr;

  void onCancelClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
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

  void onQuickConnect();

  void connectTo(wxDataViewItem profile);

};

}

