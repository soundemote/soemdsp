#include "TanhSaw.cxx"
#include "../Randoms/FlexibleRandomWalk.cxx"

array<double> sinWavetable;

void makeSinWavetable(int numIndexes = 65536)
{
    array<double> table(numIndexes);

    for (int i = 0; i < numIndexes; ++i)
    {
        table[i] = sin(double(i) / double(numIndexes) * TAU);
    }

    sinWavetable = table;
}

class AdditiveSupersawUnit
{
    double increment = 0;
    double phase = 0;
    double phaseMod = 0;
    double phaseOffset = 0;
    double amplitude = 0;
    double delayCounter = 0;
    double delaySamples = 0;
    double pan = 0.5;

    void reset()
    {
        phase = 0;
        delayCounter = 0;
    }

    double getSample()
    {
        if (delayCounter++ < delaySamples)
            return 0;

        phase += increment;
        phase = wrapPhase(phase);        
        
        int idx = int(wrapPhaseExtreme(phase+phaseMod+phaseOffset) * sinWavetable.length-1);

        return sinWavetable[clamp(idx, 0, sinWavetable.length-1)];
    }
}

class AdditiveSupersaw
{
    double frequency = 0;
    double increment = 0;
    double sampleRate = 0;
    int numHarmonics = 0;
    double samplePeriod = 0;
    double signalL = 0, signalR = 0;
    double frequencyOffset = 0;
    double frequencyOffsetCurve = 0;
    double frequencyOffsetAmp = 0;

    array<AdditiveSupersawUnit> harmonics;

    AdditiveSupersawDriftModulator driftModulator;

    void reset()
    {
        for (int i = 0; i < numHarmonics; ++i)        
            harmonics[i].reset();
    }

    void setSampleRate(double v)
    {
        sampleRate = v;
        driftModulator.setSampleRate(v);

        update();
    }

    void setFrequency(double v)
    {   
        frequency = v;
        driftModulator.setFrequency(v);
        update();
    }

    void setNumHarmonics(int v)
    {   
        int maxHarmonics = int((sampleRate * 0.5) / frequency);
        numHarmonics = int(clamp(v, 0, maxHarmonics));

        if (numHarmonics < 0)
            numHarmonics = 1;

        harmonics.resize(numHarmonics);

        for (int i = 0; i < numHarmonics; ++i)  
        {      
            harmonics[i].reset();
        
            switch (i % 2)
            {
            case 0:
                harmonics[i].pan = double(i)/double(numHarmonics);
            break;
            case 1:
                harmonics[i].pan = double(i)/double(numHarmonics);
            break;
            }
            
        }

        driftModulator.setNumHarmonics(numHarmonics);        

        update();
    }

    void setDispersion(double v)
    {
        for (int i = 0; i < numHarmonics; ++i)
        {
            dispersion = v;

            harmonics[i].delaySamples = int((numHarmonics - (i+1)) * dispersion);
            //harmonics[i].phaseOffset = fraction(delaySamples);
            harmonics[i].reset();
        }
    }

    void update()
    {
        samplePeriod = 1.0/sampleRate;
        
        for (int i = 0; i < numHarmonics; ++i)
        {
            double realFreq = frequency * (i+1);
            switch(i%2)
            {
            case 0:
                harmonics[i].increment = (realFreq + i*realFreq*frequencyOffsetAmp) * samplePeriod;
                break;
            case 1:
                harmonics[i].increment = (realFreq + -i*realFreq*frequencyOffsetAmp) * samplePeriod;
                break;
            }
        }
    }

    double getSample(double& L, double& R)
    {
        driftModulator.process(this);

        double taperAdjust = 1;
        double waveshapeAdjust = 1;
        double signal = 0;
        L = 0;
        R = 0;
        for (int i = 0; i < numHarmonics; ++i)
        {
            waveshapeAdjust = 1.0/(i+1.0);      
            
            signal += harmonics[i].getSample() * waveshapeAdjust * taperAdjust;

            L += signal * (1 - harmonics[i].pan);
            R += signal *      harmonics[i].pan;

            taperAdjust -= 1.0/double(numHarmonics);
        }

        return signal;
    }
}

class AdditiveSupersawDriftModulator
{
    double driftDetail = 0;
    double driftColor = 0;
    double driftCurve = 0;
    double driftAmplitude = 0;
    double frequency = 0;

    int numHarmonics = 0;
    array<FlexibleRandomWalk> drifts;

    void setSampleRate(double v)
    {
        for (int i = 0; i < numHarmonics; ++i)
            drifts[i].setSampleRate(v);
    }

    void setNumHarmonics(int v)
    {
        numHarmonics = v;
        drifts.resize(v);

        for (int i = 0; i < numHarmonics; ++i)
            drifts[i].setColor(driftColor);

        update();
    }

    void setFrequency(double v)
    {
        frequency = v;
        update();
    }

    void setDriftDetail(double v)
    {
        driftDetail = v;
        update();
    }

    void setDriftColor(double v)
    {
        driftColor = v;

        for (int i = 0; i < numHarmonics; ++i)
            drifts[i].setColor(driftColor);
    }

    void setDriftCurve(double v)
    {
        driftCurve = v;
    }

    void setDriftAmplitude(double v)
    {
        driftAmplitude = v;
    }

    void update()
    {
        for (int i = 0; i < numHarmonics; ++i)
            drifts[i].setDetail(frequency * (i+1) * driftDetail);
    }

    void process(AdditiveSupersaw& additiveSupersaw)
    {
        for (int i = 0; i < numHarmonics; ++i)
        {
            double value0to1 = double(i+1)/double(numHarmonics);

            double amplitudeWithCurve = rationalCurve(value0to1, driftCurve);

            additiveSupersaw.harmonics[i].phaseMod = drifts[i].getSample() * amplitudeWithCurve * driftAmplitude;
        }
    }
}