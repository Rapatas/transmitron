#include "Helpers.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <date/date.h>
#include <fmt/format.h>
#include <string_view>

using namespace std::chrono;
using namespace Rapatas::Transmitron;

wxColor Common::Helpers::colorFromNumber(size_t number) {
  // NOLINTBEGIN(readability-identifier-length)
  // NOLINTBEGIN(hicpp-signed-bitwise)

  constexpr uint8_t ByteMask = 0xFF;
  constexpr uint8_t ByteSize = std::numeric_limits<uint8_t>::digits;
  constexpr uint8_t MinValue = 150;
  constexpr uint8_t Offset = 100;

  uint8_t r = ((number >> (ByteSize * 0)) & ByteMask);
  uint8_t g = ((number >> (ByteSize * 1)) & ByteMask);
  uint8_t b = ((number >> (ByteSize * 2)) & ByteMask);

  if (r < MinValue && g < MinValue && b < MinValue) {
    r += Offset;
    g += Offset;
    b += Offset;
  }

  return {r, g, b};

  // NOLINTEND(hicpp-signed-bitwise)
  // NOLINTEND(readability-identifier-length)
}

std::string Common::Helpers::timeToString(
  const system_clock::time_point &timestamp
) {
  const auto floored = floor<milliseconds>(timestamp);
  const std::time_t nowc = std::chrono::system_clock::to_time_t(floored);
  std::tm nowtm{};
#ifndef _WIN32
  ::localtime_r(&nowc, &nowtm);
#else
  ::localtime_s(&nowtm, &nowc);
#endif // _WIN32
  std::stringstream sstream;
  sstream << std::put_time(&nowtm, "%Y-%m-%d %H:%M:%S");
  return sstream.str();
}

std::string Common::Helpers::durationToString(
  const std::chrono::milliseconds &dur
) {
  const bool negative = dur.count() < 0;
  const std::string prefix = negative ? "-" : "";
  milliseconds abs{std::abs(dur.count())};

  const auto dhours = duration_cast<hours>(abs);
  const auto dminutes = duration_cast<minutes>(abs - dhours);
  const auto dseconds = duration_cast<seconds>(abs - dhours - dminutes);
  const auto dmsec = duration_cast<milliseconds>(
    abs - dhours - dminutes - dseconds
  );

  if (dhours.count() == 0 && dminutes.count() == 0 && dseconds.count() == 0) {
    return fmt::format( //
      "{}{}ms",
      prefix,
      dmsec.count()
    );
  }

  if (dhours.count() == 0 && dminutes.count() == 0) {
    return fmt::format( //
      "{}{}.{:0<3}s",
      prefix,
      dseconds.count(),
      dmsec.count()
    );
  }

  if (dhours.count() == 0) {
    return fmt::format(
      "{}{:02}:{:02}.{:0<3}",
      prefix,
      dminutes.count(),
      dseconds.count(),
      dmsec.count()
    );
  }

  return fmt::format(
    "{}{:02}:{:02}:{:02}.{:0<3}",
    prefix,
    dhours.count(),
    dminutes.count(),
    dseconds.count(),
    dmsec.count()
  );
}

std::string Common::Helpers::timeToFilename(
  const system_clock::time_point &timestamp
) {
  const auto floored = floor<milliseconds>(timestamp);
  const std::time_t nowc = std::chrono::system_clock::to_time_t(floored);
  std::tm nowtm{};
#ifndef _WIN32
  ::localtime_r(&nowc, &nowtm);
#else
  ::localtime_s(&nowtm, &nowc);
#endif // _WIN32
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
  const std::vector<uint8_t> &bytes,
  size_t columns
) {
  constexpr size_t FixedCharacterCount = 9;
  const size_t lineWidth = columns * 4 + FixedCharacterCount;

  std::vector<uint8_t> buff(columns);
  size_t i = 0; // NOLINT
  std::string result;
  std::string line;
  line.reserve(lineWidth);

  for (i = 0; i < bytes.size(); i++) {
    if ((i % columns) == 0) {
      if (i != 0) {
        line += "  ";
        for (const auto &byte : buff) { line += static_cast<char>(byte); }
        result += line + '\n';
        line.clear();
        line.reserve(lineWidth);
      }

      line += fmt::format("{:06X}", i);
    }

    line += fmt::format(" {:02X}", bytes[i]);

    buff[i % columns] = ::isprint(bytes[i]) != 0 ? bytes[i] : '.';
  }

  while ((i % columns) != 0) {
    line += "   ";
    buff[i % columns] = ' ';
    i++;
  }

  line += "  ";
  for (const auto &byte : buff) { line += static_cast<char>(byte); }
  result += line + '\n';

  return result;
}
