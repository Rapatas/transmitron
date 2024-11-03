#ifndef _WIN32

#include "Env.hpp"

using namespace Rapatas::Transmitron::Common;

std::string Env::get(const std::string &name) {
  // NOLINTNEXTLINE
  char *result = ::getenv(name.c_str());
  if (result == nullptr) { return {}; }
  return result;
}

#endif // _WIN32
