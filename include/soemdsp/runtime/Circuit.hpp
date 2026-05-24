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
    static constexpr std::size_t blockSize{ 64 };

    std::vector<std::array<float, blockSize>> audioBuffers;

    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<Connection> connections;

    Port* output = nullptr;

    float* outputBuffer() noexcept
    {
        return output ? output->audioBuffer : nullptr;
    }

    void connect(Node& sourceNode,
                 Port& sourcePort,
                 Node& destinationNode,
                 Port& destinationPort)
    {
        connections.push_back({ &sourceNode,
                                &sourcePort,
                                &destinationNode,
                                &destinationPort });
    }

    void allocateBuffers()
    {
        audioBuffers.clear();

        std::size_t audioOutputCount = 0;

        for (auto& node : nodes)
        {
            for (auto& port : node->outputs)
            {
                if (port.type == PortType::Audio)
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
                if (port.type == PortType::Audio)
                {
                    port.audioBuffer = audioBuffers[bufferIndex].data();
                    port.audioFrames = blockSize;

                    ++bufferIndex;
                }
            }
        }
    }
    void prepare()
    {
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
                input.connected = false;
            }
        }
        for (auto& node : nodes)
        {
            for (auto& connection : connections)
            {
                if (connection.destinationNode == node.get() &&
                    connection.sourcePort &&
                    connection.destinationPort)
                {
                    connection.destinationPort->value =
                      connection.sourcePort->value;

                    connection.destinationPort->audioBuffer =
                      connection.sourcePort->audioBuffer;

                    connection.destinationPort->audioFrames =
                      connection.sourcePort->audioFrames;

                    connection.destinationPort->connected = true;
                }
            }
            node->process();
        }
    }
};

} //namespace soemdsp::runtime
