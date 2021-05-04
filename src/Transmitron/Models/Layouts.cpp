#include "Layouts.hpp"
#include <optional>

using namespace Transmitron::Models;

wxArrayString Layouts::getNames() const
{
  wxArrayString result;
  for (const auto &layout : mLayouts)
  {
    result.push_back(layout.first);
  }
  return result;
}

std::string Layouts::getUniqueName() const
{
  return "Unique name 001";
}

std::optional<std::string> Layouts::getLayout(const std::string &key) const
{
  const auto it = mLayouts.find(key);
  if (it == std::end(mLayouts))
  {
    return std::nullopt;
  }
  return it->second;
}

bool Layouts::store(const std::string &key, const std::string &layout)
{
  mLayouts[key] = layout;
  return true;
}
