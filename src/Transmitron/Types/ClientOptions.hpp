#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "Transmitron/Models/Layouts.hpp"

namespace Transmitron::Types
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

}
