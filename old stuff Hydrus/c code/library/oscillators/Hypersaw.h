#pragma once

//#include "../filters/Lowpass.h"
//#include "../filters/Highpass.h"
//#include "../modulators/LinearEnvelope.h"
//#include "../oscillators/NoiseGenerator.h"
//#include "../modulators/RandomWalk.h"
//#include "../modulators/VibratoGenerator.h"
//#include "PolyBLEP.h"

typedef struct
{
	//SineSaw osc;
	PolyBLEP osc;
	RandomWalk walkPM;
	RandomWalk walkFM;
	VibratoGenerator vibrato;
	Lowpass lpf;
	Highpass hpf;
	LinearEnvelope env;
    Lowpass smoother;
    NoiseGenerator r;

	double walkSignal;
	double filteredWalkSignal;
	double vibratoSignal;
	double phaseDistribution;
	double envelopeSignal;
    
	double targetPitch;
	double pitchOffset;

    double pitchDetuneAmount;
    double pitchDetuneCurve;
    double pitchOffsetFromDetune;

	double currentPitch;
	double currentFrequency;
	double lastPitchValue;
	double phaseOffset;
	double randomPhaseOffset;
	double randomPhaseOffsetRange;

	double PMDriftAmount;
	double PMDriftDetail;

	double targetFrequency;
	double filteredPhaseOffset;
	
	double portamentoSeconds;
	double portamentoRandomValue;
	double portamentoRandomMult;	

	double delaySecondsRandomRange;

	double attackSeconds;
	double attackSecondsRandomMult;

	double releaseSeconds;
	double releaseSecondsRandomMult;

	int idx;
	double div;
	// equal to 1 / number of sawtooths for spreading things between 0 and 1 such as oscillator phase.
	// set by Hypersaw parent
} HypersawUnit;

// Must provide the index number from 0 to 1 for each oscillator and the total oscillators being used.
void HypersawUnit_init(HypersawUnit* o, int oscillatorIdx, int totalOscillators);

void HypersawUnit_setSampleRate(HypersawUnit* o);

void HypersawUnit_reset(HypersawUnit* o);

double HypersawUnit_getSample(HypersawUnit* o);

void HypersawUnit_setPitchDetuneAmount(HypersawUnit* o, double v);
void HypersawUnit_setPitchDetuneCurve(HypersawUnit* o, double v);

void HypersawUnit_setSeed(HypersawUnit* o, int v);

void HypersawUnit_setOscMorph(HypersawUnit* o, double v);

void HypersawUnit_setPitch(HypersawUnit* o, double v);
void HypersawUnit_setPitchOffset(HypersawUnit* o, double v);
void HypersawUnit_setFilteredPhaseOffset(HypersawUnit* o, double v);
void HypersawUnit_setPortamentoSeconds(HypersawUnit* o, double v);
void HypersawUnit_setPortamentoRandomMult(HypersawUnit* o, double v);

void HypersawUnit_setPMDriftAmount(HypersawUnit* o, double v);
void HypersawUnit_setPMDriftColor(HypersawUnit* o, double v);
void HypersawUnit_setPMDriftDetail(HypersawUnit* o, double v);
void HypersawUnit_setDriftHPF(HypersawUnit* o, double v);
void HypersawUnit_setDriftLPF(HypersawUnit* o, double v);

void HypersawUnit_triggerAttack(HypersawUnit* o);
void HypersawUnit_triggerRelease(HypersawUnit* o);
void HypersawUnit_setRandomDelayRange(HypersawUnit* o, double v);
void HypersawUnit_setAttackSeconds(HypersawUnit* o, double v);
void HypersawUnit_setAttackSecondsRandomMult(HypersawUnit* o, double v);
void HypersawUnit_setReleaseSeconds(HypersawUnit* o, double v);
void HypersawUnit_setReleaseSecondsRandomMult(HypersawUnit* o, double v);

void HypersawUnit_setVibratoDesync(HypersawUnit* o, double v);
void HypersawUnit_setVibratoSpeed(HypersawUnit* o, double v);
void HypersawUnit_setVibratoRandomSpeed(HypersawUnit* o, double v);
void HypersawUnit_setVibratoRandomSpeedOffset(HypersawUnit* o, double v);
void HypersawUnit_setVibratoRange(HypersawUnit* o, double v);
void HypersawUnit_setVibratoRandomRange(HypersawUnit* o, double v);
void HypersawUnit_setVibratoRandomRangeOffset(HypersawUnit* o, double v);

void HypersawUnit_updateDetune(HypersawUnit* o);
void HypersawUnit_updatePortamento(HypersawUnit* o);
void HypersawUnit_updateAttack(HypersawUnit* o);
void HypersawUnit_updateDelay(HypersawUnit* o);
void HypersawUnit_updateRelease(HypersawUnit* o);