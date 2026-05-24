#pragma once
#include <string>
#include <vector>
#include <soemdsp/runtime/Port.hpp>
namespace soemdsp::runtime
{
struct Node
{
    std::string name;
    bool bypassed{ false };

    std::vector<Port> inputs;
    std::vector<Port> outputs;

    virtual ~Node() = default;

    virtual void prepare() {}
    virtual void process() {}
    virtual void reset() {}
};

} //namespace soemdsp::runtime
