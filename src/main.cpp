#include <wx/app.h>
#include <wx/init.h>
#include "GUI/App.hpp"
#include "Arguments.hpp"

using namespace Rapatas::Transmitron;

int main(int argc, char **argv)
{
  try
  {
    const auto args = Arguments::handleArgs(argc, argv);
    if (args.exit) { return 0; }

    auto *app = new GUI::App(args.verbose);
    wxApp::SetInstance(app);
    wxEntryStart(argc, argv);
    app->CallOnInit();

    if (!args.profileName.empty())
    {
      app->openProfile(args.profileName);
    }
    else if (!args.recordingFile.empty())
    {
      app->openRecording(args.recordingFile);
    }

    app->OnRun();
    app->OnExit();
    wxEntryCleanup();
  }
  catch (const std::exception &error)
  {
    fmt::print("{}\n", error.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
