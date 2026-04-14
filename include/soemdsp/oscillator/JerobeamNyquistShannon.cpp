#include "JerobeamNyquistShannon.h"

using namespace soemdsp::oscillator;
using std::clamp;

void JerobeamNyquistShannon::getSampleFrame(double* outL, double* outR) {
    fphas = waveshape::unipolar::trisaw(phasor.getSample(), tri);

    double stair = floor(fphas * sr) / sr;
    double phas  = clamp(blend * fmod(fphas * sr, 1.0), 0.0, 1.0) / sr + stair;

    double wave_x = phas * 2.0 - 1.0;
    double wave_y{};

    const double actualTone = [&]() {
        switch (toneMode) {
        case JerobeamNyquistShannon::None:
            return tone;
        case JerobeamNyquistShannon::Note:
            toneSmoother.setTarget(midiNote);
            return tone + toneSmoother.run();
        case JerobeamNyquistShannon::Pitch:
            toneSmoother.setTarget(pitch - 1);
            return tone + toneSmoother.run();
        case JerobeamNyquistShannon::Note_Pitch:
            toneSmoother.setTarget((pitch - 1) + midiNote);
            return tone + toneSmoother.run();
        case JerobeamNyquistShannon::Freq:
            return tone + freqToPitch;
        case JerobeamNyquistShannon::Note_Freq:
            toneSmoother.setTarget(midiNote * 0.5);
            return tone + toneSmoother.run() + freqToPitch * 0.5;
        case JerobeamNyquistShannon::Pitch_Freq:
            toneSmoother.setTarget(pitch - 1);
            return tone + toneSmoother.run() + freqToPitch;
        case JerobeamNyquistShannon::Note_Pitch_Freq:
        default:
            toneSmoother.setTarget((pitch - 1) + midiNote * 0.5);
            return tone + toneSmoother.run() + freqToPitch * 0.5;
        }
    }();

    double ps_x_PI = phaseRotator.getSample() * kTAU; /*optimization*/
    if (change(fphas) == 1) {
        wave_y = sin(actualTone * kTAU * phas + ps_x_PI);
    } else {
        wave_y = -sin(sr * kPI * phas + kPIz2) * sin(phas * (sr / 2 - actualTone) * kTAU - ps_x_PI);
    }

    *outL = wave_x;
    *outR = wave_y;

    run_gen(); // for "change" function
}
