#pragma once
#include <array>
#include <memory>
#include <vector>
#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Node.hpp>
namespace soemdsp::runtime
{
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
