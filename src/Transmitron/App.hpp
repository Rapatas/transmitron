#ifndef TRANSMITRON_APP_HPP
#define TRANSMITRON_APP_HPP

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include "Transmitron/Models/Connections.hpp"

namespace Transmitron
{

struct App :
  public wxApp
{
  bool OnInit();
private:

  unsigned mCount = 0;
  wxAuiNotebook *mNote;

  wxObjectDataPtr<Models::Connections> mConnectionsModel;

  void onPageClosed(wxBookCtrlEvent& event);
  void onPageSelected(wxBookCtrlEvent& event);

  void newConnectionTab();

  /// Empty if it fails.
  static std::string getConfigDir();
};

}

#endif // TRANSMITRON_APP_HPP
