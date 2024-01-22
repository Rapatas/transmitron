#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "GUI/Models/Layouts.hpp"

namespace Rapatas::Transmitron::GUI::Types
{

class ClientOptions
{
public:

    explicit ClientOptions() = default;
    explicit ClientOptions(std::string layout);

    static ClientOptions fromJson(const nlohmann::json &data);
    nlohmann::json toJson() const;

    std::string getLayout() const;

private:

    std::string mLayout{Models::Layouts::DefaultName};
};

} // namespace Rapatas::Transmitron::GUI::Types
