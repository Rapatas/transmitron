#include <CLI/CLI.hpp>
#include <fmt/format.h>

#include "Arguments.hpp"
#include "Common/Info.hpp"
#include "Common/Version.hpp"

Arguments Arguments::handleArgs(int argc, char **argv)
{
  const auto projectInfo = fmt::format(
    "{} {}",
    Common::Info::getProjectName(),
    Common::Info::getProjectVersion()
  );

  CLI::App args;

  Arguments result;
  result.argc = argc;
  result.argv = argv;

  args.set_version_flag("--version", projectInfo);

  auto *verboseOpt = args.add_flag(
    "--verbose",
    "Print logs"
  );

  args.add_option(
    "--profile",
    result.profileName,
    "Profile to launch"
  );

  auto *recordingFileOpt = args.add_option(
    "recording,--recording",
    result.recordingFile,
    "History recording file to load"
  );
  recordingFileOpt->option_text(".TMRC");

  try { args.parse(argc, argv); }
  catch (const CLI::ParseError &e)
  {
    args.exit(e);
    result.exit = true;
  }

  result.verbose = !verboseOpt->empty();

  return result;
}
