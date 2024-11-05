#pragma once

#include <string>
#include <vector>

namespace Rapatas::Transmitron::Common::String {

std::vector<std::string> split(const std::string &data, char delim);

std::string replace(
  const std::string &str,
  const std::string &what,
  const std::string &with
);

} // namespace Rapatas::Transmitron::Common::String
