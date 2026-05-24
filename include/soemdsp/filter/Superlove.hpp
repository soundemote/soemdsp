#include <soemdsp/random/Random.hpp>
#include <soemdsp/utility/Graph.hpp>

#include "../../../libs/RSMET/LadderFilter.h"

namespace soemdsp::filter {

class FMDFilter {
  public:
    FMDFilter()          = default;
    virtual ~FMDFilter() = default;

    /* Run this in your constructor */

    virtual void setupCommon() {
        // noiseFilter.setLowpassCutoff(20594.5); //TODO
        // noiseFilter.setHighpassCutoff(0.12491); //TODO
    }

    virtual void setSampleRate(double v) {
        sampleRate = v;
        // noiseFilter.setSampleRate(sampleRate); //TODO
    };
    virtual void setFrequency(double v) {
        frequency = v;
    } // 0 to 1
    virtual void setResonance(double v) {
        resonance = v;
    } // 0 to 1
    virtual void setNoiseAmount(double v) {
        noiseAmount = v;
    } // 0 to 1
    virtual void setNoiseSeed(int v) {
        noise.seed_ = v;
        noise.reset();
    };
    virtual void setInputAmplitude(double v) {
        inputAmplitude = v;
    } // suggested -10 to +10
    virtual double getSample(double in) {
        return in * inputAmplitude;
    };
    virtual double getFrequency() {
        return frequency;
    }

    random::LCG64 noise;
    // MonoTwoPoleBandpass noiseFilter; // TODO

  protected:
    double smoothingTime{ 0.050 };
    double sampleRate{ 44100.0 };

    double inputAmplitude{ 1.0 };
    double frequency{ .5 };
    double resonance{};
    double noiseAmount{};
};

class SuperLoveLP18 : public FMDFilter {
  public:
    SuperLoveLP18() {
        // filter.setMode(RAPT::rsLadderFilter<double, double>::LP_18); // TODO
        // dcfilter.setMode(RAPT::rsLadderFilter<double, double>::HP_18); // TODO
        // dcfilter.setCutoff(10); // TODO

        resonanceGraph.addNode(0, 0);
        resonanceGraph.addNode(1, -2.7175, -0.85, utility ::Graph::Node::Shape::EXPONENTIAL);

        noiseGraph.addNode(0, 0.00);
        noiseGraph.addNode(0.75, 0.05, -0.7, utility ::Graph::Node::Shape::EXPONENTIAL);
        noiseGraph.addNode(1, 0.10, +0.6, utility ::Graph::Node::Shape::EXPONENTIAL);

        setResonance(0);
    }

    void setFrequency(double v) override {
        FMDFilter::setFrequency(v);
        // filter.setCutoff(clamp(elan::pitch_to_freq(jmap(frequency, -12.0, +135.0)), 0.0, 0.5 * sampleRate)); //TODO
    }

    void setResonance(double v) override {
        FMDFilter::setResonance(v);
        mod = resonanceGraph.getValue(resonance);
    }

    void setNoiseAmount(double v) override {
        FMDFilter::setNoiseAmount(v);
        shape    = noiseAmount;
        noiseAmp = noiseGraph.getValue(v);
    }

    void setSampleRate(double v) override {
        sampleRate = v;

        // filter.setSampleRate(v); //TODO
        // dcfilter.setSampleRate(v); //TODO
    }

    double getSample(double in) override {
        feedbackSignal = mod * feedbackSignal + inputAmplitude * in;

        // double pm       = noiseFilter.getSample(noise.getSampleBipolar()) * noiseAmp; //TODO
        // double oscValue = -waveshape::trisaw(feedbackSignal + 0.25725 + pm, shape); //TODO

        // feedbackSignal = filter.getSample(oscValue); //TODO

        return 0; /* dcfilter.getSample(feedbackSignal) * 1.02; */ // TODO
    }

  protected:
    // RAPT::rsLadderFilter<double, double> filter; // TODO
    // RAPT::rsLadderFilter<double, double> dcfilter; // TODO
    soemdsp::utility::Graph resonanceGraph;
    soemdsp::utility::Graph noiseGraph;
    random::LCG64 NoiseGenerator;

    double shape{ 0.5 };
    double mod{ -2.7175 };
    double noiseAmp{};
    double feedbackSignal{};
};
} // namespace soemdsp::filter
