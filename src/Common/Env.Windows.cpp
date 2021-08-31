#ifdef WIN32

#include "Env.hpp"

#include <processenv.h>
#include <vector>

using namespace Common;

std::string Env::get(const std::string &name)
{
  LPCSTR lpName = name.c_str();
  const DWORD nSize = 4096;

  std::string buffer;
  buffer.resize(nSize);

  const auto size = GetEnvironmentVariableA(lpName, buffer.data(), nSize);
  if (size == 0)
  {
    return {};
  }

  return {buffer.begin(), buffer.begin() + size};
}


#endif // WIN32
