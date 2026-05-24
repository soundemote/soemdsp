#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <soemdsp/runtime/Port.hpp>
namespace soemdsp::runtime
{
struct Node
{
    std::string name;
    std::uint64_t id{ 0 };

    bool bypassed{ false };
    bool muted{ false };

    std::vector<Port> inputs;
    std::vector<Port> outputs;

    virtual ~Node() = default;

    virtual void prepare() {}
    virtual void process() {}
    virtual void reset() {}
};

} //namespace soemdsp::runtime
