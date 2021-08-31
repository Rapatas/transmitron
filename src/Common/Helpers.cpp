#include "Helpers.hpp"

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
