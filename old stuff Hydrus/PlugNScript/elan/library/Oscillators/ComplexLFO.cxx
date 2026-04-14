#include "../Randoms/SampleAndHold.cxx"
#include "../Oscillators/PolyBLEP.cxx"
#include "../Filters/Lowpass.cxx"
#include "../Filters/Highpass.cxx"
#include "../Timers/PreciseTimer.cxx"

class ComplexLFO
{
    ComplexLFO()
    {
        lpf.setFrequency(15000);
        hpf.setFrequency(5);
    }
    void reset()
    {
        rnd.reset();

        timer.reset();
        for (int i = 0; i < oscillators.length; ++i)
            oscillators[i].reset();

        lpf.reset();
        
    }

    void setLoopTime(double v)
    {
        loopTime = v;
        loopFrequency = 1 / v;
        timer.setFrequency(loopFrequency);
    }

    void setSampleRate(double v)
    {
        sampleRate = v;

        timer.setSampleRate(v);

        for (int i = 0; i < oscillators.length; ++i)
            oscillators[i].setSampleRate(v);

        update();
    }

    void setSeed(int v)
    {
        rnd.setSeed(v);
        //noise.setSeed(v+1);
    }

    void setFrequency(double v)
    {
        frequency = v;
        update();        
    }

    void setBPM(double v)
    {
        bpm = v;
        update();
    }

    void setIsSynced(bool v)
    {
        isSynced = v;
        update();
    }

    void setLowpassPitch(double v)
    {
        lpf.setFrequency(pitchToFrequency(v));
    }
    void setHighpassPitch(double v)
    {
        hpf.setFrequency(pitchToFrequency(v));
    }

    void randomizePattern()
    {
        int waveformToUse = 0;
        
        for (int i = 0; i < oscillators.length; ++i)
        {        
            int wIndex = roundToInt(rnd.getSampleUnipolar() * double(possibleWaveforms.length - 1));

            waveformToUse = possibleWaveforms[wIndex];

            oscillators[i].setWaveform(waveformToUse);

            if (i == 0)
                continue;
            
            int pIndex = rnd.getSampleUnipolar() * possiblePhaseOffsets.length - 1;
            oscillators[i].setPhaseOffset(possiblePhaseOffsets[pIndex]);

            int fIndex = roundToInt(rnd.getSampleUnipolar() * (possibleFreqMults.length - 1));

            frequencyMultipliers[i] = possibleFreqMults[fIndex];

            oscillators[i].setFrequency(oscillators[i].phasor.frequency * frequencyMultipliers[i]);            
        }

        
    }
    double output = 0;

    double getSample()
    {
        if (timer.hasTriggered())
        {
            for (int i = 0; i < oscillators.length; ++i)
                oscillators[i].reset();
        }

        double v1 =  oscillators[0].getSample() * amplitudes[0];
        double v2 =  oscillators[1].getSample() * amplitudes[1];
        double v3 =  oscillators[2].getSample() * amplitudes[2];
        double v4 =  oscillators[3].getSample() * amplitudes[3];

        double output = (v1+v2+v3+v4);
        //output += max(v1 , v2) + max(v3, v4) + min(v1 , v2) + min(v3, v4);
        //output *= .25;
        //double output = (v1 % v2 + v3 % v4) % 1;

        //oscillators[0].setPhaseOffset(output * .02);
        oscillators[1].setPhaseOffset(output * .02);
        oscillators[2].setPhaseOffset(output * .02);
        oscillators[3].setPhaseOffset(output * .02);

        timer.increment();

        output = lpf.getSample(output);
        output = hpf.getSample(output);

        return output;
    }

    void setOscillatorAmplitude(uint index, double amplitude)
    {
        if (index < 0 || index > 3)
            return;

        amplitudes[index] = amplitude;
    }
    
    void setDisableSin(bool v)
    {
        uint match = possibleWaveforms.find(WAVEFORM::SIN);

        if (v && match >= 0)
           possibleWaveforms.removeAt(match);
        else if (!v && match < 0)
            possibleWaveforms.insertAt(0, WAVEFORM::SIN);
    }
    void setDisableTri(bool v)
    {
        setDisableWaveform(WAVEFORM::MODIFIED_TRIANGLE, v);
    }
    void setDisableSaw(bool v)
    {
        setDisableWaveform(WAVEFORM::SAWTOOTH, v);
    }
    void setDisableRamp(bool v)
    {
        setDisableWaveform(WAVEFORM::RAMP, v);
    }
    void setDisableSquare(bool v)
    {
        setDisableWaveform(WAVEFORM::SQUARE, v);
    }

    private void update()
    {
        for (int i = 0; i < oscillators.length; ++i)
        {
            oscillators[i].setFrequency(frequency * frequencyMultipliers[i]);
        }
    }

    private void setDisableWaveform(int waveform, bool isDisabled)
    {
        if (waveform != WAVEFORM::SIN &&
            waveform != WAVEFORM::MODIFIED_TRIANGLE &&
            waveform != WAVEFORM::SAWTOOTH &&
            waveform != WAVEFORM::RAMP &&
            waveform != WAVEFORM::SQUARE)
            return;

        int match = possibleWaveforms.find(waveform);

        if (isDisabled && match >= 0)
            possibleWaveforms.removeAt(match);
        else
        {
            possibleWaveforms.insertLast(waveform);
            possibleWaveforms.sortAsc(1, possibleWaveforms.length - 1);
        }
    }

    double sampleRate = 0;
    double frequency = 0;
    double bpm = 0;
    double loopTime = 0;
    double loopFrequency = 0;
    bool isSynced = false;

    NoiseGenerator rnd;
    SampleAndHold snh;
    Lowpass lpf;
    Highpass hpf;

    array<double> mem;

    array<PolyBLEP> oscillators(4);
    PreciseTimer timer;
    array<double> phases = {0, 0, 0, 0};
    array<double> amplitudes = {1, 1, 1, 1};
    array<double> frequencyMultipliers = {1, 1, 1, 1};

    array<double> possiblePhaseOffsets = {0, .5};
    array<double> possibleFreqMults = {0.5, 1.0, 2.0, 4.0, 8.0, 16.0};
    array<int> possibleWaveforms = {WAVEFORM::SIN, WAVEFORM::MODIFIED_TRIANGLE, WAVEFORM::SAWTOOTH, WAVEFORM::RAMP, WAVEFORM::SQUARE};

}