#include "Transmitron/App.hpp"
#include <wx/app.h>
#include <wx/init.h>

int main(int argc, char **argv)
{
  wxApp::SetInstance(new Transmitron::App);
  wxEntryStart(argc, argv);
  wxTheApp->CallOnInit();
  wxTheApp->OnRun();
  wxTheApp->OnExit();
  wxEntryCleanup();
  return 0;
}
