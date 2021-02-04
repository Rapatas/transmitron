#ifndef HELPERS_HELPERS_HPP
#define HELPERS_HELPERS_HPP

#include <nlohmann/json.hpp>

namespace Helpers
{

template<typename T>
static std::optional<T> extract(
  const nlohmann::json &data,
  const std::string &key
);

template<>
std::optional<unsigned> extract<unsigned>(
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
std::optional<std::string> extract<std::string>(
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
std::optional<bool> extract<bool>(
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


}

#endif // HELPERS_HELPERS_HPP
