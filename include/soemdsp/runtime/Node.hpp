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
    std::string category;
    std::string description;
    std::uint64_t id{ 0 };
    std::uint64_t nextPortId{ 1 };

    bool bypassed{ false };
    bool muted{ false };

    std::vector<Port> inputs;
    std::vector<Port> outputs;

    virtual ~Node() = default;

    virtual std::string displayName() const
    {
        return category.empty()
            ? name
            : category + "/" + name;
    }

    virtual std::string summary() const
    {
        return description.empty()
            ? displayName()
            : displayName() + " - " + description;
    }

    void assignPortIds()
    {
        for (auto& input : inputs)
        {
            if (input.id == 0)
            {
                input.id = nextPortId++;
            }
        }

        for (auto& output : outputs)
        {
            if (output.id == 0)
            {
                output.id = nextPortId++;
            }
        }
    }

    virtual void prepare() {}
    virtual void process() {}
    virtual void reset() {}
};

} //namespace soemdsp::runtime
