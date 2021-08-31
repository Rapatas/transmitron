#include <algorithm>
#include <sstream>

#include "String.hpp"

using namespace Common;

std::vector<std::string> String::split(const std::string &data, char delim)
{
  std::vector<std::string> result;

  const size_t segments = 0U
    + (data.empty() ? 1U : 0U)
    + (size_t)std::count_if(
      std::begin(data),
      std::end(data),
      [&](char c)
      {
        return c == delim;
      }
    );

  result.reserve(segments);

  std::stringstream ss(data);
  std::string segment;
  while (std::getline(ss, segment, delim))
  {
    result.push_back(segment);
  }

  return result;
}
