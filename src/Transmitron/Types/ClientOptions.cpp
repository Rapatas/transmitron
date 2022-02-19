#include "ClientOptions.hpp"
#include "Common/Extract.hpp"
#include "Transmitron/Models/Layouts.hpp"

using namespace Transmitron::Types;

ClientOptions::ClientOptions(std::string layout) :
    mLayout(std::move(layout))
{}

ClientOptions ClientOptions::fromJson(const nlohmann::json &data)
{
    using namespace Common;

    std::string layout = extract<std::string>(data, "layout")
      .value_or(std::string(Models::Layouts::DefaultName));

    return ClientOptions {
        layout
    };
}

nlohmann::json ClientOptions::toJson() const
{
    return {
        {"layout", mLayout}
    };
}

std::string ClientOptions::getLayout() const
{
    return mLayout;
}

