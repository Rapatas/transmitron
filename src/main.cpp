#include <CLI/CLI.hpp>
#include <wx/app.h>
#include <wx/init.h>
#include "Transmitron/Info.hpp"
#include "Transmitron/Version.hpp"
#include "Transmitron/App.hpp"

int main(int argc, char **argv)
{
  CLI::App args;

  const auto projectInfo = fmt::format(
    "{} {}",
    getProjectName(),
    getProjectVersion()
  );
  args.set_version_flag("--version", projectInfo);

  std::string profileName;
  auto *profileNameOpt = args.add_option(
    "--profile",
    profileName,
    "Profile to launch"
  );

  // Handles the standard arguments like `help` and `version`.
  CLI11_PARSE(args, argc, argv);

  auto *app = new Transmitron::App;
  wxApp::SetInstance(app);
  wxEntryStart(argc, argv);
  app->CallOnInit();

  if (!profileNameOpt->empty())
  {
    app->openProfile(profileName);
  }

  app->OnRun();
  app->OnExit();
  wxEntryCleanup();
  return 0;
}
