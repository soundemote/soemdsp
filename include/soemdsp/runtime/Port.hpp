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
    std::string description;
    std::uint64_t id{ 0 };
    float value{ 0.0f };

    bool connected{};
    bool triggered{ false };

    float* audioBuffer{ nullptr };
    std::size_t audioFrames{ 0 };

    void* backing{ nullptr };

    bool carriesAudio() const
    {
        return type == PortType::Audio;
    }
    std::string displayName() const
    {
        return name.empty()
            ? "Port"
            : name;
    }
    std::string summary() const
    {
        return description.empty()
            ? displayName()
            : displayName() + " - " + description;
    }
    bool carriesControl() const
    {
        return type == PortType::Control;
    }
    bool carriesTrigger() const
    {
        return type == PortType::Trigger;
    }
    bool isTriggered() const
    {
        return triggered;
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
