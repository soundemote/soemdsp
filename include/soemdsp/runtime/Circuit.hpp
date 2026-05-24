#pragma once
#include <algorithm>
#include <array>
#include <memory>
#include <string_view>
#include <vector>
#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Node.hpp>
namespace soemdsp::runtime
{
using NodeId = std::uint64_t;
using ParameterId = std::string_view;

struct Circuit
{
    std::uint64_t nextNodeId{ 1 };
    std::uint64_t nextConnectionId{ 1 };

    static constexpr std::size_t blockSize{ 64 };
    std::vector<std::array<float, blockSize>> audioBuffers;

    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<Connection> connections;

    Port* output = nullptr;

    float* outputBuffer() noexcept
    {
        return output ? output->audioBuffer : nullptr;
    }
    void assignNodeIds()
    {
        for (auto& node : nodes)
        {
            if (node->id == 0)
            {
                node->id = nextNodeId++;
            }
        }
    }
    bool connect(Node& sourceNode, Port& sourcePort, Node& destinationNode, Port& destinationPort)
    {
        if (sourcePort.direction != PortDirection::Output)
        {
            return false;
        }
        if (destinationPort.direction != PortDirection::Input)
        {
            return false;
        }
        if (sourcePort.type != destinationPort.type)
        {
            const bool audioToControl =
              sourcePort.carriesAudio() &&
              destinationPort.carriesControl();
            if (!audioToControl)
            {
                return false;
            }
        }

        Connection connection;

        connection.id              = nextConnectionId++;
        connection.sourceNode      = &sourceNode;
        connection.sourcePort      = &sourcePort;
        connection.destinationNode = &destinationNode;
        connection.destinationPort = &destinationPort;

        connections.push_back(connection);

        return true;
    }
    bool disconnect(std::uint64_t connectionId)
    {
        for (auto it = connections.begin();
             it != connections.end();
             ++it)
        {
            if (it->id == connectionId)
            {
                connections.erase(it);
                return true;
            }
        }

        return false;
    }
    bool hasConnectionTo(const Port& port) const
{
    for (const auto& connection : connections)
    {
        if (connection.destinationPort == &port)
        {
            return true;
        }
    }

    return false;
}
    const Parameter* findParameter(NodeId nodeId, ParameterId parameterId) const
    {
        for (const auto& node : nodes)
        {
            if (node->id != nodeId)
            {
                continue;
            }

            for (const auto& parameter : node->parameters)
            {
                if (parameter.id == parameterId)
                {
                    return &parameter;
                }
            }

            return nullptr;
        }

        return nullptr;
    }

    Parameter* findParameter(NodeId nodeId, ParameterId parameterId)
    {
        for (auto& node : nodes)
        {
            if (node->id != nodeId)
            {
                continue;
            }

            for (auto& parameter : node->parameters)
            {
                if (parameter.id == parameterId)
                {
                    return &parameter;
                }
            }

            return nullptr;
        }

        return nullptr;
    }

    const Parameter* findParameterByName(NodeId nodeId, std::string_view name) const
    {
        for (const auto& node : nodes)
        {
            if (node->id != nodeId)
            {
                continue;
            }

            for (const auto& parameter : node->parameters)
            {
                if (parameter.name == name)
                {
                    return &parameter;
                }
            }

            return nullptr;
        }

        return nullptr;
    }

    Parameter* findParameterByName(NodeId nodeId, std::string_view name)
    {
        for (auto& node : nodes)
        {
            if (node->id != nodeId)
            {
                continue;
            }

            for (auto& parameter : node->parameters)
            {
                if (parameter.name == name)
                {
                    return &parameter;
                }
            }

            return nullptr;
        }

        return nullptr;
    }

    bool setParameterValue(NodeId nodeId, ParameterId parameterId, float value)
    {
        auto* parameter = findParameter(nodeId, parameterId);
        if (!parameter)
        {
            return false;
        }

        parameter->value =
          std::clamp(value, parameter->minValue, parameter->maxValue);

        return true;
    }

    bool setParameterValueByName(NodeId nodeId, std::string_view name, float value)
    {
        auto* parameter = findParameterByName(nodeId, name);
        if (!parameter)
        {
            return false;
        }

        parameter->value =
          std::clamp(value, parameter->minValue, parameter->maxValue);

        return true;
    }

    bool resetParameterValue(NodeId nodeId, ParameterId parameterId)
    {
        auto* parameter = findParameter(nodeId, parameterId);
        if (!parameter)
        {
            return false;
        }

        parameter->value =
          std::clamp(parameter->defaultValue, parameter->minValue, parameter->maxValue);

        return true;
    }

    bool resetParameterValueByName(NodeId nodeId, std::string_view name)
    {
        auto* parameter = findParameterByName(nodeId, name);
        if (!parameter)
        {
            return false;
        }

        parameter->value =
          std::clamp(parameter->defaultValue, parameter->minValue, parameter->maxValue);

        return true;
    }

    float getParameterNormalizedValue(NodeId nodeId, ParameterId parameterId) const
    {
        const auto* parameter = findParameter(nodeId, parameterId);
        if (!parameter)
        {
            return 0.0f;
        }

        const float range = parameter->maxValue - parameter->minValue;
        if (range == 0.0f)
        {
            return 0.0f;
        }

        return (parameter->value - parameter->minValue) / range;
    }

    float getParameterNormalizedValueByName(NodeId nodeId, std::string_view name) const
    {
        const auto* parameter = findParameterByName(nodeId, name);
        if (!parameter)
        {
            return 0.0f;
        }

        const float range = parameter->maxValue - parameter->minValue;
        if (range == 0.0f)
        {
            return 0.0f;
        }

        return (parameter->value - parameter->minValue) / range;
    }

    bool setParameterNormalizedValue(NodeId nodeId, ParameterId parameterId, float normalizedValue)
    {
        const auto* parameter = findParameter(nodeId, parameterId);
        if (!parameter)
        {
            return false;
        }

        const float range = parameter->maxValue - parameter->minValue;
        if (range == 0.0f)
        {
            return setParameterValue(nodeId, parameterId, parameter->minValue);
        }

        const float normalized =
          std::clamp(normalizedValue, 0.0f, 1.0f);
        const float value =
          parameter->minValue + (normalized * range);

        return setParameterValue(nodeId, parameterId, value);
    }

    bool setParameterNormalizedValueByName(NodeId nodeId, std::string_view name, float normalizedValue)
    {
        const auto* parameter = findParameterByName(nodeId, name);
        if (!parameter)
        {
            return false;
        }

        const float range = parameter->maxValue - parameter->minValue;
        if (range == 0.0f)
        {
            return setParameterValueByName(nodeId, name, parameter->minValue);
        }

        const float normalized =
          std::clamp(normalizedValue, 0.0f, 1.0f);
        const float value =
          parameter->minValue + (normalized * range);

        return setParameterValueByName(nodeId, name, value);
    }

    bool isParameterAtDefault(NodeId nodeId, ParameterId parameterId) const
    {
        const auto* parameter = findParameter(nodeId, parameterId);
        if (!parameter)
        {
            return false;
        }

        return parameter->value == parameter->defaultValue;
    }

    bool isParameterAtDefaultByName(NodeId nodeId, std::string_view name) const
    {
        const auto* parameter = findParameterByName(nodeId, name);
        if (!parameter)
        {
            return false;
        }

        return parameter->value == parameter->defaultValue;
    }

    bool isParameterDirty(NodeId nodeId, ParameterId parameterId) const
    {
        const auto* parameter = findParameter(nodeId, parameterId);
        if (!parameter)
        {
            return false;
        }

        return parameter->value != parameter->defaultValue;
    }

    bool isParameterDirtyByName(NodeId nodeId, std::string_view name) const
    {
        const auto* parameter = findParameterByName(nodeId, name);
        if (!parameter)
        {
            return false;
        }

        return parameter->value != parameter->defaultValue;
    }

    std::size_t parameterCount() const
    {
        std::size_t count = 0;

        for (const auto& node : nodes)
        {
            count += node->parameters.size();
        }

        return count;
    }

    std::size_t dirtyParameterCount() const
    {
        std::size_t count = 0;

        for (const auto& node : nodes)
        {
            for (const auto& parameter : node->parameters)
            {
                if (parameter.value != parameter.defaultValue)
                {
                    ++count;
                }
            }
        }

        return count;
    }

    bool hasDirtyParameters() const
    {
        for (const auto& node : nodes)
        {
            for (const auto& parameter : node->parameters)
            {
                if (parameter.value != parameter.defaultValue)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void allocateBuffers()
    {
        audioBuffers.clear();
        std::size_t audioOutputCount = 0;
        for (auto& node : nodes)
        {
            for (auto& port : node->outputs)
            {
                if (port.carriesAudio())
                {
                    ++audioOutputCount;
                }
            }
        }
        audioBuffers.resize(audioOutputCount);
        std::size_t bufferIndex = 0;
        for (auto& node : nodes)
        {
            for (auto& port : node->outputs)
            {
                if (port.carriesAudio())
                {
                    port.audioBuffer = audioBuffers[bufferIndex].data();
                    port.audioFrames = blockSize;
                    ++bufferIndex;
                }
            }
        }
    }
    void reset()
    {
        for (auto& node : nodes)
        {
            node->reset();
        }
    }
    void prepare()
    {
        assignNodeIds();
        for (auto& node : nodes)
        {
            node->assignPortIds();
        }

        allocateBuffers();
        for (auto& node : nodes)
        {
            node->prepare();
        }
    }
    void process()
    {
        for (auto& buffer : audioBuffers)
        {
            buffer.fill(0.0f);
        }
        for (auto& node : nodes)
        {
            for (auto& input : node->inputs)
            {
                input.connected   = false;
                input.triggered   = false;
                input.audioBuffer = nullptr;
                input.audioFrames = 0;
            }
        }
        for (auto& node : nodes)
        {
            for (auto& connection : connections)
            {
                if (connection.destinationNode == node.get() &&
                    connection.valid())
                {
                    if (connection.sourcePort->carriesControl())
                    {
                        connection.destinationPort->value =
                          connection.sourcePort->value;
                    }
                    connection.destinationPort->audioBuffer =
                      connection.sourcePort->audioBuffer;
                    connection.destinationPort->audioFrames =
                      connection.sourcePort->audioFrames;
                    connection.destinationPort->triggered =
                      connection.sourcePort->triggered;
                    connection.destinationPort->connected = true;
                }
            }
            if (node->muted)
            {
                continue;
            }

            if (node->bypassed)
            {
                continue;
            }

            node->process();
        }
    }
};
} //namespace soemdsp::runtime
