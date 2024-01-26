#pragma once

#include <nlohmann/json.hpp>
#include <optional>

namespace Rapatas::Transmitron::Common
{

template<typename T>
std::optional<T> extract(
  const nlohmann::json &data,
  const std::string &key
);

template<>
std::optional<unsigned> extract<unsigned>(
  const nlohmann::json &data,
  const std::string &key
);

template<>
std::optional<uint16_t> extract<uint16_t>(
  const nlohmann::json &data,
  const std::string &key
);

template<>
std::optional<std::string> extract<std::string>(
  const nlohmann::json &data,
  const std::string &key
);

template<>
std::optional<bool> extract<bool>(
  const nlohmann::json &data,
  const std::string &key
);

} // namespace Rapatas::Transmitron::Common
