#include "../../PrettyScope/Source/OpenGLOscilloscope.h"
#include <map>
#include <vector>
#include "ChaosArpModule.h"
#include "jura_framework/tools/jura_StringTools.h"

using namespace se;
using namespace lmd;
using namespace jura;
using namespace rosic;
using namespace juce;

void ChaosArp_ArpeggiatorModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("Arp_Enable", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setIsBypassed(v > 0.0); });

	p = new jura::ModulatableParameter("MidiOut_Enable", -1, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("StepSize", 0, 27, 15, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setStepLengthBars(elan::indexToTimeSigValue(v)); });

	p = new jura::ModulatableParameter("NoteQuantize", 0, 27, 18, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setNoteChangeBars(elan::indexToTimeSigValue(v)); });

	p = new jura::ModulatableParameter("NoteLength", 0, 27, 18, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setNoteLengthBars(elan::indexToTimeSigValue(v)); });

	p = new jura::ModulatableParameter("NoteLengthPercent", 0, 2, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setNoteLengthModifier(v); });

	p = new jura::ModulatableParameter("PatternResetBeats", 1, 129, 8, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setFullResetBeats(v); });

	p = new jura::ModulatableParameter("patternResetSpeedMultiplier", .1, 20000, 1, jura::Parameter::EXPONENTIAL);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setPatternResetSpeedMultiplier(v); });

	p = new jura::ModulatableParameter("OctavesAmplitude", 0, 8, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setOctaveAmplitude(v); });

	p = new jura::ModulatableParameter("PitchOffset", -48, 48, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { notePitchQuantizer.setPitchOffset(v); });

	p = new jura::ModulatableParameter("Velocity", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });
}

void ChaosArp_FilterModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("Filter_Bypass", -1, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("lpFrequency", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("lpResonance", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("lpChaos", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("hpFrequency", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("hpResonance", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("hpChaos", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("outAmp", 0, 1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { outputOscCore.setAmplitude(v); });
}

void ChaosArp_MainOscModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("bypass", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setIsBypassed(v > 0.0); });

	p = new jura::ModulatableParameter("amplitude", -1, 1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setAmplitude(v); });

	p = new jura::ModulatableParameter("WaveBend_Asym", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setWaveAsymmetrical(v); });

	p = new jura::ModulatableParameter("WaveBend_Bend", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setWaveBendAktDec(v); });

	p = new jura::ModulatableParameter("WaveBend_BendAsym", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setWaveBendOffset(v); });

	p = new jura::ModulatableParameter("WaveBend_Sigmoid", -1, 1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setWaveSigmoidAtkDec(v); });

	p = new jura::ModulatableParameter("WaveBend_SigmoidAsym", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setWaveSigmoidOffset(v); });

	p = new jura::ModulatableParameter("frequency", -1000, +1000, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setFrequencyOffset(v); });

	p = new jura::ModulatableParameter("Pitch", -36, 36, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setPitchOffset(v); });

	p = new jura::ModulatableParameter("Portamento", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setPortamento(v); });

	p = new jura::ModulatableParameter("Octave", -8, 8, 0, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setOctaveOffset(v); });

	p = new jura::ModulatableParameter("Phase", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { outputOscCore.setPhaseOffset(v); });

	//Reset
	p = new jura::ModulatableParameter("resetOnArpNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpStep", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpStep = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpPattern", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpPattern = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotArpLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotArpLegato = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnUserNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnUserNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotUserLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotUserLegato = v > 0.0; });
}

void ChaosArp_EnvelopeModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("bypass", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setIsBypassed(v > 0.0); });

	p = new jura::ModulatableParameter("amplitude", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setAmplitudeScale(v); });

	p = new jura::ModulatableParameter("PhaseInvert", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { invertModifier = v > 0 ? -1 : +1; });

	p = new jura::ModulatableParameter("Loop", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setEnableLoop(v > 0.0); });

	p = new jura::ModulatableParameter("Unipolar_Enable", -1, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { isUnipolar = v > 0.0; });

	p = new jura::ModulatableParameter("Beats_Enable", -1, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setEnableSync(v > 0.0); });

	p = new jura::ModulatableParameter("Attack", 0, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setAttack(v);; });

	p = new jura::ModulatableParameter("Decay", 0, 10, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setDecay(v); });

	p = new jura::ModulatableParameter("Sustain", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setSustainLevel(v); });

	p = new jura::ModulatableParameter("Release", 0, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setRelease(v); });

	p = new jura::ModulatableParameter("AttackCurve", -10, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setAttackCurve(v); });

	p = new jura::ModulatableParameter("DecayCurve", -10, 100, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setDecayCurve(v); });

	p = new jura::ModulatableParameter("ReleaseCurve", -10, 100, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { envelope.setReleaseCurve(v); });

	// Trigger
	p = new jura::ModulatableParameter("triggerOnArpNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnArpNote = v > 0.0; });

	p = new jura::ModulatableParameter("triggerOnArpStep", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnArpStep = v > 0.0; });

	p = new jura::ModulatableParameter("triggerOnArpPattern", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnArpPattern = v > 0.0; });

	p = new jura::ModulatableParameter("triggerIfNotArpLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerIfNotArpLegato = v > 0.0; });

	p = new jura::ModulatableParameter("triggerOnUserNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerOnUserNote = v > 0.0; });

	p = new jura::ModulatableParameter("triggerIfNotUserLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { triggerIfNotUserLegato = v > 0.0; });

	// Reset
	p = new jura::ModulatableParameter("resetOnArpNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpStep", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpStep = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpPattern", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpPattern = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotArpLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotArpLegato = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnUserNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnUserNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotUserLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotUserLegato = v > 0.0; });
}

void ChaosArp_LFOModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("bypass", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { lfo.setIsBypassed(v > 0.0); });

	p = new jura::ModulatableParameter("amplitude", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { lfo.setAmplitude(v > 0.0); });

	p = new jura::ModulatableParameter("Phase", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { lfo.setPhaseOffset(v); });

	p = new jura::ModulatableParameter("PhaseInvert", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { lfo.setPhaseInverted(v > 0.0); });

	p = new jura::ModulatableParameter("Unipolar_Enable", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { lfo.setIsUnipolar(v > 0.0); });

	p = new jura::ModulatableParameter("Tempo", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("frequency", -1000, 1000, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { lfo.setFrequencyOffset(v); });

	p = new jura::ModulatableParameter("FreqRandom", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("Amp", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("AmpRandom", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("Pitch", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("Octave", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("PitchEnable", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("WaveBend_Asym", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("WaveBend_AttackBend", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("WaveBend_DecayBend", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("WaveBend_AttackSgimoid", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("WaveBend_DecaySgimoid", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	//Reset
	p = new jura::ModulatableParameter("resetOnArpNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpStep", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpStep = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpPattern", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpPattern = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotArpLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotArpLegato = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnUserNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnUserNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotUserLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotUserLegato = v > 0.0; });
}

void ChaosArp_ChaosOscModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("rotateX", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.rotateX(v); });
	p = new jura::ModulatableParameter("rotateY", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.rotateY(v); });
	p = new jura::ModulatableParameter("rotateZ", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.rotateZ(v); });

	p = new jura::ModulatableParameter("offsetX", -10, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.lorenz.x_offset = v; });
	p = new jura::ModulatableParameter("offsetY", -10, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.lorenz.y_offset = v; });
	p = new jura::ModulatableParameter("offsetZ", -10, 10, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.lorenz.z_offset = v; });

	p = new jura::ModulatableParameter("setRho", -100, 100, 28, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setRho(v); });
	p = new jura::ModulatableParameter("setSigma", -100, 100, 10, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setSigma(v); });
	p = new jura::ModulatableParameter("setBeta", -100, 100, 8.0/3.0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setBeta(v); });

	// Main
	p = new jura::ModulatableParameter("Phase", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setPhaseOffset(v); });

	p = new jura::ModulatableParameter("PhaseInvert", 0, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setPhaseInverted(v > 0); });

	p = new jura::ModulatableParameter("Filter_LP", 0, 20000, 20000, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setLowpassCutoff(v); });

	p = new jura::ModulatableParameter("Filter_HP", 0, 20000, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setHighpassCutoff(v); });

	p = new jura::ModulatableParameter("Tempo", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setTempo(v); });

	p = new jura::ModulatableParameter("Freq", 0, 10000, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setFrequencyOffset(v); });

	p = new jura::ModulatableParameter("Pitch", -36, 36, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setPitchOffset(v); });

	p = new jura::ModulatableParameter("Octave", -8, 8, 0, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setOctaveOffset(v); });

	p = new jura::ModulatableParameter("PitchEnable", 0, 1, 0, jura::Parameter::LINEAR, 1);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setPitchEnabled(v > 0); });

	// Sends
	p = new jura::ModulatableParameter("ArpSend", 0, 1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setArpOutAmp(v); });

	p = new jura::ModulatableParameter("ArpSend_isUnipolar", 0, 1, 1, jura::Parameter::LINEAR);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setIsArpOutUnipolar(v > 0); });
	
	p = new jura::ModulatableParameter("ArpSend_PostFilterEnable", -1, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { chaosOscCore.setIsArpOutFiltered(v > 0); });
	
	//Reset
	p = new jura::ModulatableParameter("resetOnArpNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpStep", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpStep = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnArpPattern", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnArpPattern = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotArpLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotArpLegato = v > 0.0; });

	p = new jura::ModulatableParameter("resetOnUserNote", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetOnUserNote = v > 0.0; });

	p = new jura::ModulatableParameter("resetIfNotUserLegato", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	p->setValueChangeCallback([this](double v) { resetIfNotUserLegato = v > 0.0; });
}

ChaosArpModule::ChaosArpModule(CriticalSection *lockToUse, jura::MetaParameterManager * metaManagerToUse)
	: BasicModule(lockToUse, metaManagerToUse)
	, audioBuffer(lockToUse)
{
  ScopedLock scopedLock(*lock);

	setModuleLink("https://www.soundemote.com/products/chaosArp");
	setModuleName("ChaosArp");

	colorMapsPath  = PluginFileManager::workingDirectory+"/ColorMaps";

	setActiveDirectory(presetPath);
	colorMap.setActiveDirectory(colorMapsPath);

	// setup brushes
	brushes = vector<Brush*>{ &line1, &dot1, &line2, &dot2 };
	
  juce::ColourGradient g;
  g.addColour(0.00, Colour(  0,   2,  48));
	g.addColour(0.30, Colour(101, 000, 173));
  g.addColour(0.50, Colour(255, 000, 254));
	g.addColour(1.00, Colour(255, 199, 229));
  colorMap.setFromColourGradient(g);

	screenScanner.setSync(true);
	screenScanner.setNumCyclesShown(3);

	line1.lengthFade = 2048;
	line1.lenColorIntensity = 10;
	line1.intensity = 0.05f;
	line2.intensity = 0.0;
	dot2.doDraw = false;
	dot1.doDraw = false;

	BasicModule::setupParametersAndModules();
}

void ChaosArpModule::setupModulesAndModulation()
{
	MIDIMASTER.addMidiSlave(this);

	mainOscModule = new ChaosArp_MainOscModule(lock, metaParamManager);
	mainOscModule->setModuleName("MainOsc");
	addChildAudioModule(mainOscModule);

	arpModule = new ChaosArp_ArpeggiatorModule(lock, metaParamManager);
	arpModule->setModuleName("Arp");
	addChildAudioModule(arpModule);
	MIDIMASTER.addMidiSlave(&arpModule->notePitchQuantizer);

	filterModule = new ChaosArp_FilterModule(lock, metaParamManager);
	filterModule->setModuleName("FltrEnv");
	addChildAudioModule(filterModule);

	chaosOsc1Module = new ChaosArp_ChaosOscModule(lock, metaParamManager);
	chaosOsc1Module->setModuleName("ChaosOsc1");
	addChildAudioModule(chaosOsc1Module);

	chaosOsc2Module = new ChaosArp_ChaosOscModule(lock, metaParamManager);
	chaosOsc2Module->setModuleName("ChaosOsc2");
	addChildAudioModule(chaosOsc2Module);

	lfoModule = new ChaosArp_LFOModule(lock, metaParamManager);
	lfoModule->setModuleName("LFO");
	addChildAudioModule(lfoModule);

	ampEnvModule = new ChaosArp_EnvelopeModule(lock, metaParamManager);
	ampEnvModule->setModuleName("AmpEnv");
	addChildAudioModule(ampEnvModule);

	filterEnvModule = new ChaosArp_EnvelopeModule(lock, metaParamManager);
	filterEnvModule->setModuleName("FltrEnv");
	addChildAudioModule(filterEnvModule);

	arpModule->notePitchQuantizer.UserNoteOnFunction = [this]()
	{
		if (ampEnvModule->triggerOnUserNote)
			ampEnvModule->envelope.triggerOn();

		if (filterEnvModule->triggerOnUserNote)
			filterEnvModule->envelope.triggerOn();

		if (chaosOsc1Module->resetOnUserNote)
			chaosOsc1Module->reset();

		if (chaosOsc2Module->resetOnUserNote)
			chaosOsc2Module->reset();

		if (lfoModule->resetOnUserNote)
			lfoModule->reset();

		if (mainOscModule->resetOnUserNote)
			mainOscModule->outputOscCore.reset();

		mainOscModule->outputOscCore.setNote(arpModule->notePitchQuantizer.getQuantizedPitch());
	};

	arpModule->notePitchQuantizer.UserLegatoFunction = [this]()
	{
		if (ampEnvModule->triggerIfNotUserLegato)
			ampEnvModule->envelope.triggerOn();

		if (filterEnvModule->triggerIfNotUserLegato)
			filterEnvModule->envelope.triggerOn();

		if (chaosOsc1Module->resetIfNotUserLegato)
			chaosOsc1Module->reset();

		if (chaosOsc2Module->resetIfNotUserLegato)
			chaosOsc2Module->reset();

		if (lfoModule->resetIfNotUserLegato)
			lfoModule->reset();

		if (mainOscModule->resetIfNotUserLegato)
			mainOscModule->outputOscCore.reset();

		mainOscModule->outputOscCore.setNote(arpModule->notePitchQuantizer.getQuantizedPitch());
	};

	arpModule->notePitchQuantizer.arpNoteFunction = [this]()
	{
		if (ampEnvModule->triggerOnArpNote)
			ampEnvModule->envelope.triggerOn();

		if (filterEnvModule->triggerOnArpNote)
			filterEnvModule->envelope.triggerOn();

		if (chaosOsc1Module->resetOnArpNote)
			chaosOsc1Module->reset();

		if (chaosOsc2Module->resetOnArpNote)
			chaosOsc2Module->reset();

		if (lfoModule->resetOnArpNote)
			lfoModule->reset();

		if (mainOscModule->resetOnArpNote)
			mainOscModule->outputOscCore.reset();

		mainOscModule->outputOscCore.setNote(arpModule->notePitchQuantizer.getQuantizedPitch());
	};
		
	arpModule->notePitchQuantizer.ArpLegatoFunction = [this]()
	{
		if (ampEnvModule->triggerIfNotArpLegato)
			ampEnvModule->envelope.triggerOn();

		if (filterEnvModule->triggerIfNotArpLegato)
			filterEnvModule->envelope.triggerOn();

		if (chaosOsc1Module->resetIfNotArpLegato)
			chaosOsc1Module->reset();

		if (chaosOsc2Module->resetIfNotArpLegato)
			chaosOsc2Module->reset();

		if (lfoModule->resetIfNotArpLegato)
			lfoModule->reset();

		if (mainOscModule->resetIfNotArpLegato)
			mainOscModule->outputOscCore.reset();

		mainOscModule->outputOscCore.setNote(arpModule->notePitchQuantizer.getQuantizedPitch());
	};

	arpModule->notePitchQuantizer.arpStepFunction = [this]()
	{
		if (ampEnvModule->triggerOnArpStep)
			ampEnvModule->envelope.triggerOn();

		if (filterEnvModule->triggerOnArpStep)
			filterEnvModule->envelope.triggerOn();

		if (chaosOsc1Module->resetOnArpStep)
			chaosOsc1Module->reset();

		if (chaosOsc2Module->resetOnArpStep)
			chaosOsc2Module->reset();

		if (lfoModule->resetOnArpStep)
			lfoModule->reset();

		if (mainOscModule->resetOnArpStep)
			mainOscModule->outputOscCore.reset();

		mainOscModule->outputOscCore.setNote(arpModule->notePitchQuantizer.getQuantizedPitch());
	};

	arpModule->notePitchQuantizer.arpPatternFunction = [this]()
	{
		if (ampEnvModule->triggerOnArpPattern)
			ampEnvModule->envelope.triggerOn();

		if (filterEnvModule->triggerOnArpPattern)
			filterEnvModule->envelope.triggerOn();

		if (chaosOsc1Module->resetOnArpPattern)
			chaosOsc1Module->reset();

		if (chaosOsc2Module->resetOnArpPattern)
			chaosOsc2Module->reset();

		if (lfoModule->resetOnArpPattern)
			lfoModule->reset();

		if (mainOscModule->resetOnArpPattern)
			mainOscModule->reset();

		mainOscModule->outputOscCore.setNote(arpModule->notePitchQuantizer.getQuantizedPitch());
	};
	 
	arpModule->notePitchQuantizer.noteOffFunction = [this]()
	{
		ampEnvModule->envelope.triggerOff();
		filterEnvModule->envelope.triggerOff();
	};
}

void ChaosArpModule::createParameters()
{
	jura::ModulatableParameter * p;

	p = new jura::ModulatableParameter("ParameterSmoothing", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("Oversampling", -1, 1, 0, jura::Parameter::LINEAR);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });

	p = new jura::ModulatableParameter("allowInputSignal", 0, 1, 0, jura::Parameter::BOOLEAN);
	addObservedParameter(p);
	//p->setValueChangeCallback([this](double v) { triSawOsc.setAsymmetrical(v); });
}

void ChaosArpModule::setSampleRate(double v)
{
	for (auto & obj : childModules)
		obj->setSampleRate(v);

	mainOscModule->outputOscCore.setSampleRate(v);
	arpModule->notePitchQuantizer.setSampleRate(v);
	chaosOsc1Module->setSampleRate(v);
	chaosOsc2Module->setSampleRate(v);
}

void ChaosArpModule::setStateFromXml(const XmlElement& xmlState, const juce::String& stateName,
  bool markAsClean)
{
  BasicModule::setStateFromXml(xmlState, stateName, markAsClean);

  XmlElement* xmlColorMap = xmlState.getChildByName("ColorMap");
  if(xmlColorMap != nullptr)
    colorMap.setFromXml(*xmlColorMap);
}

XmlElement* ChaosArpModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
  XmlElement* xml = BasicModule::getStateAsXml(stateName, markAsClean);

  XmlElement* colorMapXml = colorMap.getAsXml();
  xml->addChildElement(colorMapXml);
  return xml;
}

double ChaosArpModule::getScannerSaw(double in)
{
  return 2 * screenScanner.getSample(in) - 1;
}

void ChaosArpModule::reset()
{
	AudioModuleWithMidiIn::reset();
	screenScanner.reset();
	mainOscModule->outputOscCore.reset();
}

void ChaosArpModule::setPitchBend(int channel, int value)
{
}

void ChaosArpModule::setChannelPressure(int channel, int value)
{
}

void ChaosArpModule::setMidiController(int channel, int controller, int value)
{
}

void ChaosArpModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
	ScopedLock scopedLock(*lock);

	if (BasicModule::isDemoTimedOut())
	{
		BasicModule::processBlockDemo(inOutBuffer, numChannels, numSamples);
		return;
	}		

	const bool doApplyModulations = modulationManager.getNumConnections() != 0;

	const bool doApplySmoothing = smoothingManager->needsSmoothing();

	double x, y, xt, yt, ss;

	bool onedimensional = false;

	for (int n = 0; n < numSamples; n++)
	{
		if (doApplySmoothing)
			smoothingManager->updateSmoothedValues();

		if (doApplyModulations)
			modulationManager.applyModulations();

		// retrieve input point:
		x = inOutBuffer[0] ? inOutBuffer[0][n] : 0.0;
		y = inOutBuffer[1] ? inOutBuffer[1][n] : 0.0;

		/*PROCESS SAMPLE*/

		double arp1Val = chaosOsc1Module->getSample();
		double arp2Val = chaosOsc2Module->getSample();
		double arpSendValue = arp1Val + arp2Val;

		arpModule->notePitchQuantizer.setPitchValue(arpSendValue);
		arpModule->notePitchQuantizer.increment();

		x = y = mainOscModule->outputOscCore.getSample() * ampEnvModule->envelope.getSample();

		/****************/

		xt = arp1Val;
		yt = arp2Val;

		audioBuffer.processSampleFrame(&xt, &yt);		

		if (onedimensional)
		{
			// for pitch analysis
			switch (0)
			{
			case 0: ss = getScannerSaw(xt+yt); break;
			case 1: ss = getScannerSaw(yt); break;
			case 2: ss = getScannerSaw(xt); break;
			case 3: ss = getScannerSaw(xt+yt); break;
			}

			// store transformed point in circular buffer:
			audioBuffer.bufX[audioBuffer.bufIndex] = (float)ss;
			audioBuffer.bufY[audioBuffer.bufIndex] = (float)xt;
			audioBuffer.bufY2[audioBuffer.bufIndex] = (float)yt;
		}
		else
		{
			// store transformed point in circular buffer:
			audioBuffer.bufX[audioBuffer.bufIndex] = (float)xt;
			audioBuffer.bufY[audioBuffer.bufIndex] = (float)yt;
		}

		if (inOutBuffer[0]) { inOutBuffer[0][n] = x; }
		if (inOutBuffer[1]) { inOutBuffer[1][n] = y; }		

		audioBuffer.updateStartIndex();
		MIDIMASTER.incrementPitchGlide();
	}

	audioBuffer.processBlock(inOutBuffer, numChannels, numSamples);
}
