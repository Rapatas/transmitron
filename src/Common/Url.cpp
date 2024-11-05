#include "Url.hpp"

#include <algorithm>
#include <stdexcept>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

constexpr uint8_t NibbleFirst = 0xF0;
constexpr uint8_t NibbleSeccond = 0x0F;
constexpr uint8_t AsciiOffsetDigit = 48;
constexpr uint8_t AsciiOffsetChar = 55;
constexpr uint8_t MaxSingleDigit = 9;

using namespace Rapatas::Transmitron::Common;

bool Url::encodable(char value) {
  // clang-format off
  return !(false // NOLINT
    || isalnum(value) != 0
    || value == '-'
    || value == '_'
    || value == ','
    || value == '.'
  );
  // clang-format on
}

bool Url::isHexChar(char value) {
  return isdigit(value) != 0 || (value >= 'A' && value <= 'F');
}

std::string Url::encode(const std::string &data) {
  const auto encodeCharCount = static_cast<size_t>(std::count_if(
    std::begin(data),
    std::end(data),
    [](char value) { return encodable(value); }
  ));

  const auto length = (encodeCharCount * 3) + (data.length() - encodeCharCount);
  std::string result;
  result.resize(length);

  size_t index = 0;
  for (const auto &value : data) {
    if (!encodable(value)) {
      result[index++] = value;
      continue;
    }

    // NOLINTBEGIN(hicpp-signed-bitwise)
    const uint8_t first = (value & NibbleFirst) >> 4;
    const uint8_t second = value & NibbleSeccond;

    result[index++] = '%';
    result[index++] = first <= MaxSingleDigit
      ? static_cast<char>(first + AsciiOffsetDigit)
      : static_cast<char>(first + AsciiOffsetChar);
    result[index++] = second <= MaxSingleDigit
      ? static_cast<char>(second + AsciiOffsetDigit)
      : static_cast<char>(second + AsciiOffsetChar);
    // NOLINTEND(hicpp-signed-bitwise)
  }

  return result;
}

std::string Url::decode(const std::string &data) {
  const auto encodeCharCount = static_cast<size_t>(
    std::count(std::begin(data), std::end(data), '%')
  );

  const auto length = (data.length() - encodeCharCount * 3) + encodeCharCount;
  std::string result;
  result.resize(length);

  auto charToNibble = [](char value) {
    if (!isHexChar(value)) {
      const auto msg = fmt::format(
        "Unexpected non-printable ASCII char '0x{:X}' '{}'",
        value,
        value
      );
      throw std::runtime_error(msg.c_str());
    }
    return (value >= '0' && value <= '9')
      ? static_cast<uint8_t>(value - AsciiOffsetDigit)
      : static_cast<uint8_t>(value - AsciiOffsetChar);
  };

  size_t index = 0;
  for (auto it = data.begin(); it != data.end();) {
    if (*it != '%') {
      result[index++] = *it;
      it++;
      continue;
    }
    ++it;
    const uint8_t first = charToNibble(*it++);
    const uint8_t second = charToNibble(*it++);
    result[index++] = static_cast<char>((first << 4U) + second);
  }

  return result;
}
