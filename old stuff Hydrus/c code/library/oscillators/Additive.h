double BiasFunction(double harmonic, int totalHarmonics, double bias);

typedef struct AdditiveMaster AdditiveMaster;

typedef struct AdditiveFxDrift
{
	double driftDetail;
	double driftColor;
	double driftCurve;
	double driftAmplitude;
	double frequency;
	int numHarmonics;	

	AdditiveMaster* master;

	RandomWalk drifts[ADDITIVE_MAX_HARMONICS];
} AdditiveFxDrift;

void AdditiveFxDrift_init(AdditiveFxDrift* o, AdditiveMaster* master);
void AdditiveFxDrift_reset(AdditiveFxDrift* o);

void AdditiveFxDrift_setSampleRate(AdditiveFxDrift* o);

void AdditiveFxDrift_setNumHarmonics(AdditiveFxDrift* o, int v);

void AdditiveFxDrift_setFrequency(AdditiveFxDrift* o, double v);

void AdditiveFxDrift_setDriftDetail(AdditiveFxDrift* o, double v);
void AdditiveFxDrift_setDriftColor(AdditiveFxDrift* o, double v);
void AdditiveFxDrift_setDriftCurve(AdditiveFxDrift* o, double v);
void AdditiveFxDrift_setDriftAmplitude(AdditiveFxDrift* o, double v);

void AdditiveFxDrift_update(AdditiveFxDrift* o);
void AdditiveFxDrift_apply(AdditiveFxDrift* o, AdditiveMaster* m);

typedef struct AdditiveFxDetune
{
	double dispersion;
} AdditiveFxDetune;

typedef struct AdditiveFxPan
{
	double panCurve;
	double panBias;
} AdditiveFxPan;

typedef struct AdditiveUnit
{
	double globalPhaseOffset;
	double increment;

	double frequency;

	double phase;
	double phaseMod;
	double randomPhase;
	double randomPhaseAmp;
	double skewPhase;

	double amplitude;
	double delayCounter;
	double delaySamples;
	double pan;

	double freqMod;

	double randomPitch;
	double randomFreq;

	AdditiveMaster* master;
} AdditiveUnit;

void AdditiveUnit_DBG(AdditiveUnit* o)
{
	LOGF(o->increment)
	LOGF(o->frequency)
	LOGF(o->phase)
	LOGF(o->phaseMod)
	LOGF(o->randomPhaseAmp)
	LOGF(o->skewPhase)
	LOGF(o->amplitude)
	LOGF(o->delayCounter)
	LOGF(o->delaySamples)
	LOGF(o->pan)
	LOGF(o->freqMod)
	LOGF(o->randomPitch)
	LOGF(o->randomFreq)
}

void AdditiveUnit_init(AdditiveMaster* m, AdditiveUnit* o);
void AdditiveUnit_reset(AdditiveUnit* o);
double AdditiveUnit_getSample(AdditiveUnit* o);

enum StereoMode { MONO, PSEUDOSTEREO, DOUBLEMONO };

typedef struct AdditiveMaster
{
	int stereoMode;
	double pitch;
	double frequency;
	double increment;
	double sampleRate;
	int numHarmonics;
	int userNumHarmonics;
	double signalL;
	double signalR;
	double frequencyOffset;
	double frequencyOffsetCurve;
	double frequencyOffsetAmp;	
	double ampCurve;
	double randomPhaseAmp;
	enum WAVEFORM waveform;
	double morph;
	bool allHarmonicsSameAmp;
	double globalPhaseOffset;

	AdditiveUnit harmonics[ADDITIVE_MAX_HARMONICS];

	AdditiveFxDrift driftFx;
} AdditiveMaster;

void AdditiveMaster_DBG(AdditiveMaster* o)
{
	FUNC
	LOGI(o->stereoMode)
	LOGF(o->pitch)
	LOGF(o->frequency)
	LOGF(o->increment)
	LOGI(o->numHarmonics)
	LOGI(o->userNumHarmonics)
	LOGF(o->signalL)
	LOGF(o->signalR)
	LOGF(o->frequencyOffset)
	LOGF(o->frequencyOffsetCurve)
	LOGF(o->frequencyOffsetAmp)
	LOGF(o->ampCurve)
}
void AdditiveMaster_init(AdditiveMaster* o);
void AdditiveMaster_reset(AdditiveMaster* o);
void AdditiveMaster_setSampleRate(AdditiveMaster* o);
void AdditiveMaster_setStereoMode(AdditiveMaster* o, int v);
void AdditiveMaster_setNumHarmonics(AdditiveMaster* o, int v);
void AdditiveMaster_setDispersion(AdditiveMaster* o, double v);
void AdditiveMaster_update(AdditiveMaster* o);
double AdditiveMaster_getSample(AdditiveMaster* o);
