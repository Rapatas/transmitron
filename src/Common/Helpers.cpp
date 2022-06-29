#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

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
  const system_clock::time_point &timestamp,
  const std::string &format
) {
  const std::time_t timestampC = system_clock::to_time_t(timestamp);
  const std::tm timestampTm = *std::localtime(&timestampC);
  std::stringstream ss;
  ss << std::put_time(&timestampTm, format.c_str());

  const std::string &ms{"ms"};
  if (true // NOLINT
    && format.length() >= ms.length()
    && (0 == format.compare(format.length() - ms.length(), ms.length(), ms))
  ) {
    const auto secs = time_point_cast<seconds>(timestamp);
    const auto fraction = timestamp - secs;
    const auto millis = duration_cast<milliseconds>(fraction).count();
    ss << "." << millis;
  }

  return ss.str();
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
