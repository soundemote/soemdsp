#pragma once

#include "semath.hpp"

#include <array>

namespace soemdsp::midi {

struct StateVariables {

    int chan_{};   // 0 to 15
    int note_{};   // 0 to 127
    int noteid_{}; // starts at 0
    int sample_{}; // timestamp in samples of the midi event for per-block scheduling into per-sample

    double velocity_{};   // 0.0 to 1.0
    double aftertouch_{}; // 0.0 to 1.0
    double pressure_{};   // 0.0 to 1.0
    double pitchbend_{};  // -1.0 to 1.0

    int ccIdx_{};  // 0 to 127
    double ccVal_{}; // 0.0 to 1.0
};

struct Status : public StateVariables {

    enum class Message {
        note_off,
        note_on,
        all_notes_off,
        controller, // continuous controller
        pitchbend,
        aftertouch,
        pressure, // channel pressure
        program   // program change
    } message_;

    struct State : public StateVariables {
        Message message_{};
        std::array<double, 128> ccArray_{};

        void writeToData(uint8_t* data, uint32_t* blockSampleTime) {
            *blockSampleTime = sample_;

            data[0] = 0; // clear bits
            data[0] |= chan_; // bitwise or lower bits

            switch (message_) {
            case Message::note_on:
                data[0] = 0x90 | (data[0] & 0x0F);
                data[1] = static_cast<uint8_t>(note_);
                data[2] = static_cast<uint8_t>(round(velocity_ * 127.0));
                break;
            case Message::note_off:
                data[0] = 0x80 | (data[0] & 0x0F);
                data[1] = static_cast<uint8_t>(note_);
                data[2] = static_cast<uint8_t>(round(velocity_ * 127.0));
                break;
            case Message::aftertouch:
                data[0] = 0xA0 | (data[0] & 0x0F);
                data[1] = static_cast<uint8_t>(note_);
                data[2] = static_cast<uint8_t>(round(aftertouch_ * 127.0));
                break;
            case Message::controller:
                data[0] = 0xB0 | (data[0] & 0x0F);
                data[1] = static_cast<uint8_t>(ccIdx_);
                data[2] = static_cast<uint8_t>(round(ccVal_ * 127.0));
                break;
            case Message::pressure:
                data[0] = 0xD0 | (data[0] & 0x0F);
                data[1] = static_cast<uint8_t>(round(pressure_ * 127.0));
                break;
            case Message::pitchbend:
                data[0] = 0xE0 | (data[0] & 0x0F);
                int discretePitchBend{ roundToInt(pitchbend_ * 8192.0 + 8192.0) };
                data[1] = discretePitchBend % 128; // get leftover from multiple of 128
                data[2] = static_cast<uint8_t>(discretePitchBend / 128); // get multiple of 128
                break;
            }            
        }
    };

    std::array<State, 16> stateByChannel_;

    State state_; // saved state;

    void read(int16_t /*port_index*/, const uint8_t data[3], uint32_t perBlockSampleTime) {
        auto msg  = data[0] & 0xF0;
        auto chan = data[0] & 0x0F;
        chan_     = chan;
        sample_ = state_.sample_ = perBlockSampleTime;
        state_.message_ = Message{};
        switch (msg) {
        case 0x90:
            message_ = state_.message_ = Message::note_on;
            note_ = state_.note_ = data[1];
            velocity_ = state_.velocity_ = toDouble(data[2]) / 127.0;
            return;
        case 0x80:
            message_ = state_.message_ = Message::note_off;
            note_ = state_.note_ = data[1];
            velocity_ = state_.velocity_ = toDouble(data[2]) / 127.0;
            return;
        case 0xA0:
            message_ = state_.message_ = Message::aftertouch;
            note_ = state_.note_ = data[1];
            aftertouch_ = state_.aftertouch_ = data[2] / 127.0;
            return;
        case 0xB0:
            message_ = state_.message_ = Message::controller;
            ccIdx_ = state_.ccIdx_ = data[1];
            ccVal_ = state_.ccVal_ = toDouble(data[2]) / 127.0;
            return;
        case 0xD0:
            message_ = state_.message_ = Message::pressure;
            pressure_ = state_.pressure_ = toDouble(data[1]) / 127.0;
            return;
        case 0xE0:
            message_ = state_.message_ = Message::pitchbend;
            pitchbend_ = state_.pitchbend_ = toDouble(data[1] + data[2] * 128) / 8192.0 - 1.0;
            return;
        }

        // save state
        stateByChannel_[chan_] = state_;
    }
};

const std::array<std::string, 12> noteNames  = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
const std::array<std::string, 12> noteSharps = { "", "#", "", "#", "", "", "#", "", "#", "", "#", "" };

const std::array<std::string, 128> ccNames = {
    "CC 0 'Bank Select'",
    "CC 1 'Modwheel'",
    "CC 2 'Breath'",
    "CC 3",
    "CC 4 'Foot'",
    "CC 5 'Portamento'",
    "CC 6 'MSB'",
    "CC 7 'Volume'",
    "CC 8 'Stereo Pan'",
    "CC 9",
    "CC 10 'Mono Pan'",
    "CC 11 'Expression'",
    "CC 12 'Effect'",
    "CC 13 'Effect'",
    "CC 14",
    "CC 15",
    "CC 16",
    "CC 17",
    "CC 18",
    "CC 19",
    "CC 20",
    "CC 21",
    "CC 22",
    "CC 23",
    "CC 24",
    "CC 25",
    "CC 26",
    "CC 27",
    "CC 28",
    "CC 29",
    "CC 30",
    "CC 31",
    "CC 32",
    "CC 33",
    "CC 34",
    "CC 35",
    "CC 36",
    "CC 37",
    "CC 38",
    "CC 39",
    "CC 40",
    "CC 41",
    "CC 42",
    "CC 43",
    "CC 44",
    "CC 45",
    "CC 46",
    "CC 47",
    "CC 48",
    "CC 49",
    "CC 50",
    "CC 51",
    "CC 52",
    "CC 53",
    "CC 54",
    "CC 55",
    "CC 56",
    "CC 57",
    "CC 58",
    "CC 59",
    "CC 60",
    "CC 61",
    "CC 62",
    "CC 63",
    "CC 64 'Sustain'",
    "CC 65 'Portamento'",
    "CC 66 'Sostenuto'",
    "CC 67 'Soft Pedal'",
    "CC 68 'Legato'",
    "CC 69 'Hold'",
    "CC 70 'Generic'",
    "CC 71 'Timbre/Resonance'",
    "CC 72 'Release'",
    "CC 73 'Attack'",
    "CC 74 'Cutoff'",
    "CC 75 'Generic'",
    "CC 76 'Generic'",
    "CC 77 'Generic'",
    "CC 78 'Generic'",
    "CC 79 'Generic'",
    "CC 80 'Generic Switch'",
    "CC 81 'Generic Switch'",
    "CC 82 'Generic Switch'",
    "CC 83 'Generic Switch'",
    "CC 84 'Portamento Amount'",
    "CC 85",
    "CC 86",
    "CC 87",
    "CC 88 'Velocity 2'",
    "CC 89",
    "CC 90",
    "CC 91 'Reverb'",
    "CC 92 'Chorus'",
    "CC 93 'Detune'",
    "CC 94 'Phaser'",
    "CC 95 '+1 Data Increment'",
    "CC 96 '-1 Data Decrement'",
    "CC 97 'LSB NRPN 6 38 96 97'",
    "CC 98 'MSB NRPN 6 38 96 97'",
    "CC 99 'LSB RPN 6 38 96 97'",
    "CC 100 'MSB RPN 6 38 96 97'",
    "CC 101",
    "CC 102",
    "CC 103",
    "CC 104",
    "CC 105",
    "CC 106",
    "CC 107",
    "CC 108",
    "CC 109",
    "CC 110",
    "CC 111",
    "CC 112",
    "CC 113",
    "CC 114",
    "CC 115",
    "CC 116",
    "CC 117",
    "CC 118",
    "CC 119",
    "CC 120 'All Sound Off'",
    "CC 121 'Reset All Controllers'",
    "CC 122 'Local On/Off Switch'",
    "CC 123 'All Notes Off'",
    "CC 124 'Omni Mode Off'",
    "CC 125 'Omni Mode On'",
    "CC 126 'Mono Mode'",
    "CC 127 'Poly Mode'"
};

}
