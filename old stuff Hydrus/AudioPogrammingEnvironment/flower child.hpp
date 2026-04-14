#include "../../RS-MET/Misc/APE Scripts/rapt_for_ape.cpp"

#include "../../SoundemoteFramework/se_framework/APE/se_framework_for_ape.cpp"

#include <effect.h>
#include <algorithm>

using namespace ape;

using RAPT::rsSinCos;
using RAPT::rsNodeBasedFunction;
using RAPT::rsFunctionNode;

GlobalData(FlowerChild, "Bread and butter analog filter");


double pToF(double v)
{
	return 440.0*( pow(2.0, (v-69.0)/12.0) );
}

class OnePoleLowpass
{	
public:
	OnePoleLowpass()
	{
		//f.setMode(RAPT::rsOnePoleFilter<double, double>::LOWPASS_BLT);
		f.setMode(RAPT::rsOnePoleFilter<double, double>::LOWPASS_IIT);
	}
	
	void setSampleRate(double v)
	{
		f.setSampleRate(v);
	}
	
	void setFrequency(double v)
	{
		f.setCutoff(v);
	}
	
	double getSample(double v)
	{
		return f.getSample(v);
	}

protected:	

	RAPT::rsOnePoleFilter<double, double> f;

};

class FMDFilter
{
public:
	FMDFilter() = default;
	virtual ~FMDFilter() = default;

	/* Run this in your constructor */

	virtual void setSampleRate(double v)
	{
		sampleRate = v;
	};
	virtual void setFrequency(double v) { frequency = v; } // 0 to 1
	virtual void setResonance(double v) { resonance = v; } // 0 to 1
	virtual void setChaosAmount(double v) { chaosAmount = v; } // 0 to 1
	virtual void setInputAmplitude(double v) { inputAmplitude = v; } // suggested -10 to +10
	virtual double getSample(double in)
	{
		return in * inputAmplitude;
	};
	virtual double getFrequency() { return frequency; }
	
protected:
	double smoothingTime = 0.050;
	double sampleRate = 44100;

	double inputAmplitude = 1;
	double frequency = .5;
	double resonance = 0;
	double chaosAmount = 0;
};

class FlowerChild : public FMDFilter, public Effect
{
public:

	Param<float> paramCutoff    { "masterPitch", Range(0, 1) };
	Param<float> paramInput     { "InputGain",   Range(0, 6) };
	Param<float> paramOutput    { "OutputGain",  Range(0, 5) };
	Param<float> paramResonance { "Resonance",   Range(0, 1) };
	Param<float> paramNoise     { "Noise",       Range(0, 1) };
	
	FlowerChild() 
	{	
		noiseGraph.addNode(0.0, 0.0);
		noiseGraph.addNode(0.8, 0.1);
		noiseGraph.addNode(1.0, 1.0);
		
		noiseGraph.setNodeShapeType(2, rsFunctionNode<double>::RATIONAL);
		noiseGraph.setNodeShapeParameter(2, 0.0);
	
		double x = 0; // Non-resonant

		phaseModGraph.addNode(x, 0.0);
		sineAmpGraph.addNode(x, 4.44777);
		sineToSquareGraph.addNode(x, 0.6792);
		clipLevelGraph.addNode(x, 4);

		x = 0.5; // Resonant

		phaseModGraph.addNode(x, -0.017446);
		sineAmpGraph.addNode(x, 8.6687);
		sineToSquareGraph.addNode(x, 0.9552);
		clipLevelGraph.addNode(x, 4);

		x = 0.6; // Self-oscillating

		phaseModGraph.addNode(x, -0.017575);
		sineAmpGraph.addNode(x, 8.6687);
		sineToSquareGraph.addNode(x, 0.9552);
		clipLevelGraph.addNode(x, 4);

		x = 1.0; // Feedback

		phaseModGraph.addNode(x, -0.0147);
		sineAmpGraph.addNode(x, 2);
		sineToSquareGraph.addNode(x, 0.001);		
		
		clipLevelGraph.addNode(0.0, 4);
		clipLevelGraph.addNode(0.7, 4);
		clipLevelGraph.addNode(1.0, 2);
		clipLevelGraph.setNodeShapeType(1, rsFunctionNode<double>::RATIONAL);
		clipLevelGraph.setNodeShapeType(2, rsFunctionNode<double>::RATIONAL);
		clipLevelGraph.setNodeShapeParameter(1, 0.0);
		clipLevelGraph.setNodeShapeParameter(2, 0.6);
		
		phaseModGraph.setNodeShapeType(1, rsFunctionNode<double>::RATIONAL);
		sineAmpGraph.setNodeShapeType(1, rsFunctionNode<double>::RATIONAL);
		sineToSquareGraph.setNodeShapeType(1, rsFunctionNode<double>::RATIONAL);
				
		phaseModGraph.setNodeShapeType(2, rsFunctionNode<double>::RATIONAL);
		sineAmpGraph.setNodeShapeType(2, rsFunctionNode<double>::RATIONAL);
		sineToSquareGraph.setNodeShapeType(2, rsFunctionNode<double>::RATIONAL);
		
		phaseModGraph.setNodeShapeType(3, rsFunctionNode<double>::RATIONAL);
		sineAmpGraph.setNodeShapeType(3, rsFunctionNode<double>::RATIONAL);
		sineToSquareGraph.setNodeShapeType(3, rsFunctionNode<double>::RATIONAL);
		
		phaseModGraph.setNodeShapeParameter(1, 0.9);
		sineAmpGraph.setNodeShapeParameter(1, 0.9);
		sineToSquareGraph.setNodeShapeParameter(1, 0.9);
				
		phaseModGraph.setNodeShapeParameter(2, 0.0);
		sineAmpGraph.setNodeShapeParameter(2, 0.0);
		sineToSquareGraph.setNodeShapeParameter(2, 0.0);
		
		phaseModGraph.setNodeShapeParameter(3, 0.6);
		sineAmpGraph.setNodeShapeParameter(3, 0.6);
		sineToSquareGraph.setNodeShapeParameter(3, 0.6);
		
		paramCutoff = 0.5;
		paramInput = 3.5;
		paramOutput = 1.0;
		paramResonance = 0.5;
		
		feedback = 0.0;
		fmAmount = pToF(-48.377);
	}
	
	double ellipsoid(double x, double squareToSine)
	{
		double s;
		double c;
		rsSinCos(6.28318530717958647693 * x, &s, &c);
				
		x = pow(c, 2) + pow(s * squareToSine, 2);
		
		return (c / sqrt(x));			
	}	
	
	void setInputAmplitude(double v) override
	{
		FMDFilter::setInputAmplitude(v);
	}

	void setFrequency(double v) override
	{
		masterPitch = jmap(v, -120.0, 105.0);
		masterFrequency = pToF(masterPitch);
		lpf1.setFrequency(pToF(jmap(masterPitch, -120.0, 120.0, 90.0, 180.0)));
		lpf2.setFrequency(pToF(jmap(masterPitch, -120.0, 120.0, 80.0, 130.0)));
	}	

	void setResonance(double v) override
	{
		pmAmount     = phaseModGraph.getValue(v);
		sineAmp      = sineAmpGraph.getValue(v);
		sineToSquare = sineToSquareGraph.getValue(v);
		clipLevel    = std::min(sineAmp, clipLevelGraph.getValue(v));
		noiseReduction = noiseGraph.getValue(v);
	}

	void setChaosAmount(double v) override
	{
		FMDFilter::setChaosAmount(v * 4);
	}

	void setSampleRate(double v) override
	{
		FMDFilter::setSampleRate(v);

		lpf1.setSampleRate(v);
		lpf2.setSampleRate(v);

		phasor.setSampleRate(v);
	}
		
	double getSample(double in) override
	{
		in = feedback + std::clamp<double>(-1 * in * inputAmplitude, -clipLevel, +clipLevel);		
		
		double f = masterFrequency * in * fmAmount;
		double n = masterFrequency * noise.getSampleBipolar() * chaosAmount * noiseReduction;
		
		phasor.setFrequency(f + n);		
		phasor.increment();		
		phasorOut = phasor.getBipolarValue() + pmAmount * feedback;
				
		ellipseOut = sineAmp * ellipsoid(phasorOut, sineToSquare);
		
		feedback = lpf1.getSample(ellipseOut);
		feedback = lpf2.getSample(feedback);
	
		return feedback * 0.35;
	}

protected:

	// Resonance Graphs
	rsNodeBasedFunction<double> phaseModGraph;
	rsNodeBasedFunction<double> sineAmpGraph;
	rsNodeBasedFunction<double> sineToSquareGraph;
	rsNodeBasedFunction<double> clipLevelGraph;
	
	// Cutoff Graphs
	rsNodeBasedFunction<double> noiseGraph;

	// Objects
	OnePoleLowpass lpf1, lpf2;	
	Phasor phasor;
	elan::NoiseGenerator noise;
	
	// Internal states
	double phasorOut, ellipseOut, feedback;
	
	// Parameter states
	double masterPitch;
	double masterFrequency;
	double fmAmount;
	double pmAmount;
	double sineAmp;
	double sineToSquare;
	double clipLevel;
	double noiseReduction;
	
	void start(const IOConfig& cfg) override
	{
		setSampleRate(cfg.sampleRate);
	}
		
	void process(ape::umatrix<const float> ins, ape::umatrix<float> outs, size_t numFrames) override
	{
		setInputAmplitude(paramInput);		
		setResonance(paramResonance);
		setFrequency(paramCutoff);
		setChaosAmount(paramNoise);
		
    	const auto numChannels = sharedChannels();
    	
    	for(size_t n = 0; n < numFrames; ++n)
    	{
      		float x = getSample(ins[0][n]) * paramOutput;
      		      		
      		for(size_t c = 0; c < numChannels; ++c)
        		outs[c][n] = x;                 
    	}

	}
};