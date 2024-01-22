#pragma once

#include "Common/Log.hpp"
#include <string>

namespace Rapatas::Transmitron
{

struct Arguments
{
  bool exit = false;

  int argc;
  char **argv;

  std::string profileName;
  std::string recordingFile;
  bool verbose = false;

  static Arguments handleArgs(int argc, char **argv);
};

} // namespace Rapatas::Transmitron
