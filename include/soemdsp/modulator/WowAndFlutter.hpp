#pragma once

#include "soemdsp/oscillator/PolyBLEP.hpp"
#include "soemdsp/random/FlexibleRandomWalk.hpp"

namespace soemdsp::modulator {
class WowAndFlutter {
  public:
    oscillator::PolyBLEP wowOsc_;
    modulator::FlexibleRandomWalk flutterNoise_;

    WowAndFlutter() {
        wowOsc_.increment_       = 1.0 / 44100.0;
        flutterNoise_.method_    = magic_enum::enum_integer<FlexibleRandomWalk::Method>(FlexibleRandomWalk::Method::fixed_steps);
        flutterNoise_.frequency_ = 1.0;
        flutterNoise_.jitter_    = 0.01;
        flutterNoise_.sampleRateChanged();
        flutterNoise_.jitterChanged();
    }

    void sampleRateChanged() {
        flutterNoise_.sampleRateChanged();
    }

    Wire<double> wowAmp_;
    Wire<double> flutterAmp_;
    double out_;

    double run() {
        out_ = wowOsc_.run() * wowAmp_ + flutterNoise_.run() * flutterAmp_;
        return out_;
    }
};

// class WowAndFlutterCrossfade {
//   public:
//     PolyBLEP wowOsc_;
//     FlexibleRandomWalk flutterNoise_;
//
//     WowAndFlutter() {
//         wowOsc_.increment_ = 1.0 / 44100.0;
//         flutterNoise_.method_ =
//         magic_enum::enum_integer<FlexibleRandomWalk::Method>(FlexibleRandomWalk::Method::fixed_steps);
//         flutterNoise_.frequency_ = 1.0;
//         flutterNoise_.jitter_    = 0.01;
//         flutterNoise_.sampleRateChanged();
//         flutterNoise_.jitterChanged();
//     }
//
// #pragma region UPDATE
//     void sampleRateChanged() { flutterNoise_.sampleRateChanged(); }
//
//     Wire<double> wowFlutterCrossfade_; // -1 to +1
//     double out_;
// #pragma endregion
//
//     double run() {
//         SE_ASSERT(
//           wowFlutterCrossfade_ >= 0.0 && wowFlutterCrossfade_ <= 1.0, "vibratoDriftCrossfade must be between 0 and
//           1");
//
//         out_ = linearCrossfade(wowOsc_.run(), flutterNoise_.run(), wowFlutterCrossfade_);
//         return out_;
//     }
// };
} // namespace soemdsp::modulator
