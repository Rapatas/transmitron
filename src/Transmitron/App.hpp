#ifndef TRANSMITRON_APP_HPP
#define TRANSMITRON_APP_HPP

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include "Transmitron/Models/Profiles.hpp"

namespace Transmitron
{

class App :
  public wxApp
{
public:

  bool OnInit() override;

private:

  size_t mCount = 0;
  wxAuiNotebook *mNote;
  bool mDarkMode = false;

  wxObjectDataPtr<Models::Profiles> mProfilesModel;

  void onPageClosing(wxBookCtrlEvent& event);
  void onPageSelected(wxBookCtrlEvent& event);

  void createProfilesTab(size_t index);

  /// Empty if it fails.
  static std::string getConfigDir();
};

}

DECLARE_APP(Transmitron::App)

#endif // TRANSMITRON_APP_HPP
