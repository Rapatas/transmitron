#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

#include <date/date.h>
#include <fmt/format.h>

#include "Helpers.hpp"

using namespace std::chrono;
using namespace Rapatas::Transmitron;

wxColor Common::Helpers::colorFromNumber(size_t number)
{
  constexpr uint8_t MinColorChannel = 100;
  constexpr uint8_t ByteMask = 0xFF;
  constexpr uint8_t ByteSize = std::numeric_limits<uint8_t>::digits;

  // NOLINTBEGIN(readability-identifier-length)
  // NOLINTBEGIN(google-readability-casting)
  // NOLINTBEGIN(hicpp-signed-bitwise)
  uint8_t r = ((number >> (ByteSize * 0)) & ByteMask);
  uint8_t g = ((number >> (ByteSize * 1)) & ByteMask);
  uint8_t b = ((number >> (ByteSize * 2)) & ByteMask);

  if (r < MinColorChannel) { r = (uint8_t)(r + MinColorChannel); }
  if (g < MinColorChannel) { g = (uint8_t)(g + MinColorChannel); }
  if (b < MinColorChannel) { b = (uint8_t)(b + MinColorChannel); }

  return {r, g, b};
  // NOLINTEND(hicpp-signed-bitwise)
  // NOLINTEND(google-readability-casting)
  // NOLINTEND(readability-identifier-length)
}

std::string Common::Helpers::timeToString(
  const system_clock::time_point &timestamp
) {
  const auto floored = floor<milliseconds>(timestamp);
  const std::time_t nowc = std::chrono::system_clock::to_time_t(floored);
  std::tm nowtm{};
  ::localtime_r(&nowc, &nowtm);
  std::stringstream sstream;
  sstream << std::put_time(&nowtm, "%Y-%m-%d %H:%M:%S");
  return sstream.str();
}

std::string Common::Helpers::timeToFilename(
  const system_clock::time_point &timestamp
) {
  const auto floored = floor<milliseconds>(timestamp);
  const std::time_t nowc = std::chrono::system_clock::to_time_t(floored);
  std::tm nowtm{};
  ::localtime_r(&nowc, &nowtm);
  std::stringstream sstream;
  sstream << std::put_time(&nowtm, "%Y%m%dT%H%M%S");
  return sstream.str();
}

std::chrono::system_clock::time_point Common::Helpers::stringToTime(
  const std::string &line
) {
  system_clock::time_point timestamp;
  std::stringstream sstream(line);
  sstream >> date::parse("%F %T", timestamp);
  return timestamp;
}

std::string Common::Helpers::hexDump(
  const std::vector<uint8_t>& bytes,
  size_t columns
) {
  constexpr size_t FixedCharacterCount = 9;
  const size_t lineWidth = columns * 4 + FixedCharacterCount;

  std::vector<uint8_t> buff(columns);
  size_t i = 0; // NOLINT
  std::string result;
  std::string line;
  line.reserve(lineWidth);

  for (i = 0; i < bytes.size(); i++)
  {
    if ((i % columns) == 0)
    {
      if (i != 0)
      {
        line += "  ";
        for (const auto &byte : buff)
        {
          line += static_cast<char>(byte);
        }
        result += line + '\n';
        line.clear();
        line.reserve(lineWidth);
      }

      line += fmt::format("{:06X}", i);
    }

    line += fmt::format(" {:02X}", bytes[i]);

    buff[i % columns] = ::isprint(bytes[i]) != 0
      ? bytes[i]
      : '.';
  }

  while ((i % columns) != 0)
  {
    line += "   ";
    buff[i % columns] = ' ';
    i++;
  }

  line += "  ";
  for (const auto &byte : buff)
  {
    line += static_cast<char>(byte);
  }
  result += line + '\n';

  return result;
}
