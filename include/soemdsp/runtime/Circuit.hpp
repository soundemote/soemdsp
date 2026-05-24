#pragma once
#include <array>
#include <memory>
#include <vector>
#include <soemdsp/runtime/Connection.hpp>
#include <soemdsp/runtime/Node.hpp>
namespace soemdsp::runtime
{
struct Graph
{
    static constexpr std::size_t blockSize{ 64 };

    std::vector<std::array<float, blockSize>> audioBuffers;

    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<Connection> connections;

    void connect(Node& sourceNode,
             Port& sourcePort,
             Node& destinationNode,
             Port& destinationPort)
{
    connections.push_back({
        &sourceNode,
        &sourcePort,
        &destinationNode,
        &destinationPort
    });
}

    void process()
    {
        audioBuffers.clear();
        audioBuffers.reserve(nodes.size());
        for (auto& node : nodes)
        {
        //Pull connected values into this node before it runs
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
                }
            }
            for (auto& output : node->outputs)
            {
                if (output.type == PortType::Audio)
                {
                    audioBuffers.emplace_back();

                    output.audioBuffer = audioBuffers.back().data();
                    output.audioFrames = blockSize;
                }
            }

            node->process();
        }
    }
};
} //namespace soemdsp::runtime
