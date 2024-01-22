#pragma once

#include <chrono>

#include <wx/colour.h>

namespace Rapatas::Transmitron::Common::Helpers
{

wxColor colorFromNumber(size_t number);

std::string timeToFilename(
  const std::chrono::system_clock::time_point &timestamp
);

std::string timeToString(
  const std::chrono::system_clock::time_point &timestamp
);

std::chrono::system_clock::time_point stringToTime(const std::string &line);

std::string hexDump(const std::vector<uint8_t>& bytes, size_t columns);

} // namespace Rapatas::Transmitron::Common::Helpers
