#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
namespace soemdsp::runtime
{
enum class PortType : std::uint8_t
{
    Control,
    Audio,
    Trigger
};
enum class PortDirection : std::uint8_t
{
    Input,
    Output
};
struct Port
{
    Port() = default;

    Port(PortType t,
         PortDirection d = PortDirection::Input)
      : type(t)
      , direction(d)
    {
    }

    PortType type{ PortType::Control };
    PortDirection direction{ PortDirection::Input };

    std::string name;
    float value{ 0.0f };

    bool connected{};

    float* audioBuffer{ nullptr };
    std::size_t audioFrames{ 0 };

    void* backing{ nullptr };

    bool isAudio() const
    {
        return type == PortType::Audio;
    }
    bool isControl() const
    {
        return type == PortType::Control;
    }
    bool hasAudio() const
    {
        return audioBuffer != nullptr;
    }

    bool hasValue() const
    {
        return connected || value != 0.0f;
    }
};
} //namespace soemdsp::runtime
