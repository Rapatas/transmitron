#ifndef TRANSMITRON_APP_HPP
#define TRANSMITRON_APP_HPP

#include <spdlog/spdlog.h>
#include <wx/wx.h>
#include <wx/aui/auibook.h>

#include "Transmitron/Models/Profiles.hpp"
#include "Transmitron/Models/Layouts.hpp"

namespace Transmitron
{

class App :
  public wxApp
{
public:

  explicit App();

  bool OnInit() override;

private:

  std::shared_ptr<spdlog::logger> mLogger;
  size_t mCount = 0;
  wxFrame *mFrame = nullptr;
  wxAuiNotebook *mNote = nullptr;
  bool mDarkMode = false;

  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  const wxFontInfo LabelFontInfo;

  void onPageClosing(wxBookCtrlEvent& event);
  void onPageSelected(wxBookCtrlEvent& event);

  void createProfilesTab(size_t index);
  void createSettingsTab();

  /// Empty if it fails.
  static std::string getConfigDir();
};

}

DECLARE_APP(Transmitron::App)

#endif // TRANSMITRON_APP_HPP
