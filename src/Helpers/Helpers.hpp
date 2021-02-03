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

}

#endif // HELPERS_HELPERS_HPP
