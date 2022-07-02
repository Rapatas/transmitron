#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

#include <date/date.h>
#include <fmt/format.h>

#include "Helpers.hpp"

using namespace std::chrono;

wxColor Common::Helpers::colorFromNumber(size_t number)
{
  constexpr uint8_t MinColorChannel = 100;
  constexpr uint8_t ByteMask = 0xFF;
  constexpr uint8_t ByteSize = std::numeric_limits<uint8_t>::digits;

  uint8_t r = ((number >> (ByteSize * 0)) & ByteMask);
  uint8_t g = ((number >> (ByteSize * 1)) & ByteMask);
  uint8_t b = ((number >> (ByteSize * 2)) & ByteMask);

  if (r < MinColorChannel) { r = (uint8_t)(r + MinColorChannel); }
  if (g < MinColorChannel) { g = (uint8_t)(r + MinColorChannel); }
  if (b < MinColorChannel) { b = (uint8_t)(r + MinColorChannel); }

  return {r, g, b};
}

std::string Common::Helpers::timeToString(
  const system_clock::time_point &timestamp
) {
  return date::format("%F %T", floor<milliseconds>(timestamp));
}

std::string Common::Helpers::timeToFilename(
  const system_clock::time_point &timestamp
) {
  return date::format("%Y%m%dT%H%M%S", floor<seconds>(timestamp));
}

std::chrono::system_clock::time_point Common::Helpers::stringToTime(
  const std::string &line
) {
  system_clock::time_point tp;
  std::stringstream ss(line);
  ss >> date::parse("%F %T", tp);
  return tp;
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
          line += (char)byte;
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
    line += (char)byte;
  }
  result += line + '\n';

  return result;
}
