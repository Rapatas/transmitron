#include "ClientOptions.hpp"

#include "Common/Extract.hpp"
#include "GUI/Models/Layouts.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Types;

ClientOptions::ClientOptions(std::string layout) :
  mLayout(std::move(layout)) //
{}

ClientOptions ClientOptions::fromJson(const nlohmann::json &data) {
  using namespace Common;

  const auto layoutOpt = extract<std::string>(data, "layout");
  const auto layout = layoutOpt.value_or(
    std::string(Models::Layouts::DefaultName)
  );

  return ClientOptions{layout};
}

nlohmann::json ClientOptions::toJson() const { return {{"layout", mLayout}}; }

std::string ClientOptions::getLayout() const { return mLayout; }
