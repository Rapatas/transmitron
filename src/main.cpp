#include "Transmitron/App.hpp"
#include <wx/app.h>
#include <wx/init.h>

int main(int argc, char **argv)
{
  auto *app = new Transmitron::App;
  wxApp::SetInstance(app);
  wxEntryStart(argc, argv);
  app->CallOnInit();
  app->OnRun();
  app->OnExit();
  wxEntryCleanup();
  return 0;
}
