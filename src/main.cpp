#include <wx/app.h>
#include <wx/init.h>
#include "Transmitron/App.hpp"
#include "Arguments.hpp"

int main(int argc, char **argv)
{
  try
  {
    const auto args = Arguments::handleArgs(argc, argv);
    if (args.exit) { return 0; }

    auto *app = new Transmitron::App(args.verbose);
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
