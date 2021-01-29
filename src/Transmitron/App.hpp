#ifndef TRANSMITRON_APP_HPP
#define TRANSMITRON_APP_HPP

#include <wx/wx.h>
#include <wx/aui/auibook.h>

namespace Transmitron
{

struct App :
  public wxApp
{
  bool OnInit();
private:

  unsigned mCount = 0;
  wxAuiNotebook *mNote;

  void onPageClosed(wxBookCtrlEvent& event);
  void onPageSelected(wxBookCtrlEvent& event);

  void newConnectionTab();
};

}

#endif // TRANSMITRON_APP_HPP
