#include "Extract.hpp"

using namespace Rapatas::Transmitron;

template<>
std::optional<unsigned> Common::extract<unsigned>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::number_unsigned
  ) {
    return std::nullopt;
  }
  return it->get<unsigned>();
}

template<>
std::optional<uint16_t> Common::extract<uint16_t>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::number_unsigned
  ) {
    return std::nullopt;
  }
  return it->get<uint16_t>();
}

template<>
std::optional<std::string> Common::extract<std::string>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::string
  ) {
    return std::nullopt;
  }
  return it->get<std::string>();
}

template<>
std::optional<bool> Common::extract<bool>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::boolean
  ) {
    return std::nullopt;
  }
  return it->get<bool>();
}
