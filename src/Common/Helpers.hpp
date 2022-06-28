#ifndef COMMON_HELPERS_HPP
#define COMMON_HELPERS_HPP

#include <chrono>

#include <wx/colour.h>

namespace Common
{
namespace Helpers
{

wxColor colorFromNumber(size_t number);

std::string timeToString(
  const std::chrono::system_clock::time_point &timestamp,
  const std::string &format
);

}
}

#endif // COMMON_HELPERS_HPP
