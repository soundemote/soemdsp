#include <iostream>
#include <memory>
#include <vector>

#include <soemdsp/delay/PingPongDelay.hpp>

using soemdsp::delay::PingPongDelay;

int main()
{
    auto delay = std::make_unique<PingPongDelay>();
    double mem[PingPongDelay::kControlCount]{};
    std::vector<float> bufL(PingPongDelay::kMaxBufferSamples, 0.0f);
    std::vector<float> bufR(PingPongDelay::kMaxBufferSamples, 0.0f);

    delay->mem = mem;
    delay->base = 0;
    delay->setBuffers(bufL.data(), bufR.data(), PingPongDelay::kMaxBufferSamples);
    delay->initDefaults();

    std::cout << "[DSP OBJECT PING PONG ATOM]\n";
    std::cout << "kParams count: " << PingPongDelay::kControlCount << "\n";
    std::cout << "baseSeconds: " << delay->tempoBpm_() << " bpm meta ok\n";

    delay->process(0.5);
    const double a1 = delay->outL_;
    delay->syncControlParams();
    delay->process(0.0);
    delay->lpfFrequency_() = 2000.0;
    delay->syncControlParams();
    delay->process(0.0);
    std::cout << "process outL: " << a1 << "\n";
    std::cout << "control sync ok\n";
    return 0;
}
