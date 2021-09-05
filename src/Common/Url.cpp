#include "Url.hpp"

#include <algorithm>
#include <stdexcept>

#include <fmt/format.h>

constexpr uint8_t NibbleFirst = 0xF0;
constexpr uint8_t NibbleSeccond = 0x0F;
constexpr uint8_t AsciiOffsetDigit = 48;
constexpr uint8_t AsciiOffsetChar = 55;
constexpr uint8_t MaxSingleDigit = 9;

using namespace Common;

bool Url::encodable(char c)
{
  return !(false // NOLINT
    || isalnum(c) != 0
    || c == '-'
    || c == '_'
    || c == ','
    || c == '.'
  );
}

bool Url::isHexChar(char c)
{
  return isdigit(c) == 1 || (c >= 'A' && c <= 'F');
}

std::string Url::encode(const std::string &data)
{
  const auto encodeCharCount = (size_t)std::count_if(
    std::begin(data),
    std::end(data),
    [](char c)
    {
      return encodable(c);
    }
  );

  const size_t length = (encodeCharCount * 3) + (data.length() - encodeCharCount);
  std::string result;
  result.resize(length);

  size_t i = 0;
  for (const auto &c : data)
  {
    if (!encodable(c))
    {
      result[i++] = c;
      continue;
    }

    const uint8_t a = (c & NibbleFirst) >> 4;
    const uint8_t b = c & NibbleSeccond;

    result[i++] = '%';
    result[i++] = a <= MaxSingleDigit
      ? (char)(a + AsciiOffsetDigit)
      : (char)(a + AsciiOffsetChar);
    result[i++] = b <= MaxSingleDigit
      ? (char)(b + AsciiOffsetDigit)
      : (char)(b + AsciiOffsetChar);
  }

  return result;
}

std::string Url::decode(const std::string &data)
{
  const auto encodeCharCount = (size_t)std::count(
    std::begin(data),
    std::end(data),
     '%'
  );

  const size_t length = (data.length() - encodeCharCount * 3) + encodeCharCount;
  std::string result;
  result.resize(length);

  auto charToNibble = [](char c)
  {
    if (!isHexChar(c))
    {
      const auto msg = fmt::format(
        "Unexpected printable ASCII char '0x{:X}'",
        (uint8_t)c
      );
      throw std::runtime_error(msg.c_str());
    }
    return (c >= '0' && c <= '9')
      ? (uint8_t)(c - AsciiOffsetDigit)
      : (uint8_t)(c - AsciiOffsetChar);
  };

  size_t i = 0;
  for (auto it = data.begin(); it != data.end(); )
  {
    if (*it != '%')
    {
      result[i++] = *it;
      it++;
      continue;
    }
    ++it;
    const uint8_t a = charToNibble(*it++);
    const uint8_t b = charToNibble(*it++);
    result[i++] = (char)((a << 4) + b);
  }

  return result;
}
