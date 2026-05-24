#pragma once

#include <string>
#include <vector>

#include <soemdsp/runtime/Port.hpp>

namespace soemdsp::runtime {

struct Node {
    std::string name;

    std::vector<Port> inputs;
    std::vector<Port> outputs;

    virtual ~Node() = default;

    virtual void process() {}
};

} // namespace soemdsp::runtime
