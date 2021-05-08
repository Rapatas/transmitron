#ifndef TRANSMITRON_APP_HPP
#define TRANSMITRON_APP_HPP

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

  bool OnInit() override;

private:

  unsigned mCount = 0;
  wxAuiNotebook *mNote;
  bool mDarkMode = false;

  wxObjectDataPtr<Models::Profiles> mProfilesModel;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  void onPageClosed(wxBookCtrlEvent& event);
  void onPageSelected(wxBookCtrlEvent& event);

  void newConnectionTab();

  /// Empty if it fails.
  static std::string getConfigDir();
};

}

DECLARE_APP(Transmitron::App)

#endif // TRANSMITRON_APP_HPP
