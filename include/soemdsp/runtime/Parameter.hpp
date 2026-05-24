#pragma once

#include <string>

namespace soemdsp::runtime
{

struct Parameter
{
    std::string id;
    std::string name;
    std::string description;

    float value{ 0.0f };
    float defaultValue{ 0.0f };
    float minValue{ 0.0f };
    float maxValue{ 1.0f };

    bool visible{ true };
    bool automatable{ true };
};

} // namespace soemdsp::runtime
