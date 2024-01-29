#include <algorithm>
#include <sstream>

#include "String.hpp"

using namespace Rapatas::Transmitron::Common;

std::vector<std::string> String::split(const std::string &data, char delim)
{
  std::vector<std::string> result;

  const size_t segments = 0U
    + (data.empty() ? 1U : 0U)
    + static_cast<size_t>(std::count_if(
      std::begin(data),
      std::end(data),
      [&](char value)
      {
        return value == delim;
      }
    ));

  result.reserve(segments);

  std::stringstream sstream(data);
  std::string segment;
  while (std::getline(sstream, segment, delim))
  {
    result.push_back(segment);
  }

  return result;
}
