#include <CLI/CLI.hpp>
#include <cstdlib>
#include <exception>
#include <wx/app.h>
#include <wx/init.h>
#include "Common/Info.hpp"
#include "Common/Version.hpp"
#include "Transmitron/App.hpp"

int main(int argc, char **argv)
{
  try
  {
    CLI::App args;

    const auto projectInfo = fmt::format(
      "{} {}",
      Common::getProjectName(),
      Common::getProjectVersion()
    );
    args.set_version_flag("--version", projectInfo);

    std::string profileName;
    auto *profileNameOpt = args.add_option(
      "--profile",
      profileName,
      "Profile to launch"
    );

    std::string recordingFile;
    auto *recordingFileOpt = args.add_option(
      "recording,--recording",
      recordingFile,
      "History recording file to load"
    );
    recordingFileOpt->option_text(".TMRC");

    try { args.parse(argc, argv); }
    catch (const CLI::ParseError &e) { return args.exit(e); }

    auto *app = new Transmitron::App;
    wxApp::SetInstance(app);
    wxEntryStart(argc, argv);
    app->CallOnInit();

    if (!profileNameOpt->empty())
    {
      app->openProfile(profileName);
    }
    else if (!recordingFileOpt->empty())
    {
      app->openRecording(recordingFile);
    }

    app->OnRun();
    app->OnExit();
    wxEntryCleanup();
  }
  catch (std::exception &e)
  {
    fmt::print("{}\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
