#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include "Common/Log.hpp"
#include <string>

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

#endif // ARGUMENTS_HPP
