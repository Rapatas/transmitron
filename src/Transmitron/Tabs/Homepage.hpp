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

  wxFontInfo mLabelFont;
  int mOptionsHeight;

  std::shared_ptr<spdlog::logger> mLogger;

  // Profiles.
  wxPanel *mProfiles = nullptr;
  wxDataViewCtrl *mProfilesCtrl = nullptr;
  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  // Recordings.
  wxPanel *mRecordings = nullptr;

  // Buttons.
  wxPanel *mProfileButtons = nullptr;
  wxBoxSizer *mProfileButtonsSizer = nullptr;
  wxButton *mConnect = nullptr;

  void onProfileActivated(wxDataViewEvent &event);
  void onProfileSelected(wxDataViewEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onCancelClicked(wxCommandEvent &event);
  void onNewProfileClicked(wxCommandEvent &event);
  void onProfileContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onProfileDelete(wxCommandEvent &event);
  void onLayoutAdded(Events::Layout &event);
  void onLayoutRemoved(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);
  void onRecordingOpen(wxCommandEvent &event);

  void setupProfiles(wxPanel *parent);
  void setupRecordings(wxPanel *parent);
  void setupProfileButtons(wxPanel *parent);
};

}

#endif // TRANSMITRON_TABS_HOMEPAGE_HPP
