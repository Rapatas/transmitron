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

std::string String::replace(
  const std::string& str,
  const std::string& what,
  const std::string& with
) {
  if (what.empty()) { return str; }
  auto result = str;
  size_t start = 0;
  while((start = result.find(what, start)) != std::string::npos)
  {
    result.replace(start, what.length(), with);
    start += with.length();
  }
  return result;
}
