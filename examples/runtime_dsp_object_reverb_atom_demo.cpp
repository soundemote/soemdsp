#include <iostream>
#include <memory>

#include <soemdsp/delay/Reverb.hpp>

using soemdsp::delay::Reverb;

int main()
{
    // Reverb owns a multi-megabyte delay pool — keep it on the heap.
    auto reverb = std::make_unique<Reverb>();
    double mem[Reverb::kControlCount]{};
    reverb->mem = mem;
    reverb->base = 0;
    reverb->bindChildren();
    reverb->initDefaults();
    reverb->reset();

    std::cout << "[DSP OBJECT REVERB ATOM]\n";
    std::cout << "numDelays: " << reverb->numDelays_() << "\n";
    std::cout << "echoTime: " << reverb->echoTimeSeconds_() << "\n";

    reverb->process(0.5, -0.25);
    std::cout << "process #1 L/R: " << reverb->outL_ << " / " << reverb->outR_ << "\n";

    const double echoBefore = reverb->echoTimeSeconds_();
    reverb->syncControlParams();
    std::cout << "echoTime stable: "
              << (reverb->echoTimeSeconds_() == echoBefore ? "true" : "false")
              << "\n";

    reverb->echoTimeSeconds_() = 0.1;
    reverb->syncControlParams();
    reverb->process(0.5, -0.25);
    std::cout << "after echoTime change out L: " << reverb->outL_ << "\n";

    reverb->mix_() = 1.0;
    reverb->syncControlParams();
    reverb->process(0.0, 0.0);
    std::cout << "mix-only poke still processes: true\n";
    std::cout << "wet-ish out L: " << reverb->outL_ << "\n";

    return 0;
}
