#include "Connection.hpp"
#include "Homepage.hpp"

#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/wx.h>

struct Transmitron :
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
