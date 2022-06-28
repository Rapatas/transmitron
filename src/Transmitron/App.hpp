#ifndef TRANSMITRON_APP_HPP
#define TRANSMITRON_APP_HPP

#include <spdlog/spdlog.h>
#include <wx/wx.h>
#include <wx/aui/auibook.h>

#include "Transmitron/Events/Recording.hpp"
#include "Transmitron/Models/Profiles.hpp"
#include "Transmitron/Models/Layouts.hpp"

namespace Transmitron
{

class App :
  public wxApp
{
public:

  explicit App();

  bool openProfile(const std::string &profileName);

  bool OnInit() override;
  int FilterEvent(wxEvent &event) override;

private:

  std::shared_ptr<spdlog::logger> mLogger;
  size_t mCount = 0;
  wxFrame *mFrame = nullptr;
  wxAuiNotebook *mNote = nullptr;
  bool mDarkMode = false;
  int mOptionsHeight = 0;

  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  const wxFontInfo LabelFontInfo;

  void onPageClosing(wxBookCtrlEvent &event);
  void onPageSelected(wxBookCtrlEvent &event);
  void onKeyDownControlW();
  void onKeyDownControlT();
  void onRecordingSave(Events::Recording &event);
  void onRecordingOpen(Events::Recording &event);

  void createProfilesTab(size_t index);
  void createSettingsTab();
  void setupIcon();

  std::filesystem::path createConfigDir();
  std::filesystem::path createCacheDir();
  std::filesystem::path getExecutablePath();
  std::filesystem::path getInstallPrefix();

  void openProfile(wxDataViewItem item);
  void openRecording(const wxString &filename);

  int calculateOptionHeight();
};

}

DECLARE_APP(Transmitron::App)

#endif // TRANSMITRON_APP_HPP
