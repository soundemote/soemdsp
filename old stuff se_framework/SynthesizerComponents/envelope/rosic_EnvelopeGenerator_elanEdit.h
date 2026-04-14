#pragma once

class CurveEnvelope
{
public:
	CurveEnvelope()
	{
		env.setLoopMode(true);
		env.setLoopStartIndex(2);
		env.setLoopEndIndex(3);
		recalculateBreakpoints();
	}
	virtual ~CurveEnvelope() = default;

	void setSampleRate(double v) { env.setSampleRate(v); }

	double getSample() { return env.getSample(); }

	void setIsBypassed(bool v)
	{
		isBypassed = v;

		if (isBypassed)
			env.noteOff();
	}

	void triggerOn(bool doReset = true)
	{
		if (isBypassed)
			return;

		env.noteOn(!doReset);
	}
	void triggerOff()
	{
		env.noteOff();
	}

	bool isSilent() { return env.endIsReached; }

	void setEnableSync(bool v)
	{
		env.setSyncMode(v);
	}
	void setBPM(double v)
	{
		env.setBeatsPerMinute(v);
	}

	void setEnableLoop(bool v)
	{
		isLooping = v;

		if (isLooping)
		{
			env.setLoopStartIndex(0);
			env.setLoopEndIndex(1);
		}
		else
		{
			env.setLoopStartIndex(2);
			env.setLoopEndIndex(3);
		}

		recalculateSustainLevel();
	}

	void recalculateSustainLevel()
	{
		if (isLooping)
			env.setBreakpointLevel(2, 0);
		else
			env.setBreakpointLevel(2, sustain_level*amp_scale);

		env.setBreakpointLevel(3, env.getBreakpointLevel(2));
	}

	void setAmplitudeScale(double x)
	{
		amp_scale = x;
		env.setBreakpointLevel(1, amp_scale);
		env.setBreakpointLevel(2, sustain_level*amp_scale);
		env.setBreakpointLevel(3, sustain_level*amp_scale);
	}

	// velocity is 0 to 1, influence of 0 means velocity has no effect and the envelope opens fully. Influence of 1 means velocity controlls the envelope completely. Influence of -1 means
	void setVelocity(double velocity, double influence = 1.0)
	{
		jassert(velocity <= 1);
		jassert(velocity >= 0);
		jassert(influence <= 1);
		jassert(influence >= -1);

		double scale = jmap<double>(velocity, 1 - abs(influence), +1.0);

		if (influence < 0)
			scale *= -1;

		setAmplitudeScale(scale);
	}

	void setAttack(double x)
	{
		attack = x;
		recalculateBreakpoints();
	}

	void setDecay(double x)
	{
		decay = x;
		recalculateBreakpoints();
	}

	void setSustainLevel(double v)
	{
		sustain_level = v;
		recalculateSustainLevel();
	}

	void setRelease(double v)
	{
		release = v;
		recalculateBreakpoints();
	}

	/*
	 0 = Linear
	>0 = Logarithmic
	<0 = Exponential
	recommend -10 to +10 linear, 1 default
	*/
	void setAttackCurve(double v)
	{
		env.setBreakpointShape(1, getShape(v));
		env.setBreakpointShapeAmount(1, abs(v));
	}

	/*
 0 = Linear
>0 = Exponential
<0 = Logarithmic
recommend -10 to +10 linear, 1 default
*/
	void setDecayCurve(double v)
	{
		env.setBreakpointShape(2, getShape(v));
		env.setBreakpointShapeAmount(2, abs(v));
	}

	/*
 0 = Linear
>0 = Exponential
<0 = Logarithmic
recommend -10 to +10 linear, 1 default
*/
	void setReleaseCurve(double v)
	{
		env.setBreakpointShape(4, getShape(v));
		env.setBreakpointShapeAmount(4, abs(v));
	}

	double getAmplitudeScale()
	{
		return amp_scale;
	}

	rosic::BreakpointModulator env;

protected:

	void recalculateBreakpoints()
	{
		// zero start
		env.setBreakpointTime(1, attack);

		// attack
		env.setBreakpointLevel(1, amp_scale);

		// decay
		env.setBreakpointTime(2, attack + decay);

		// sustain
		env.setBreakpointTime(3, attack + decay + sustainLoopTime);

		// release
		env.setBreakpointTime(4, attack + decay + sustainLoopTime + release);
	}

	rosic::ModBreakpoint::shapes getShape(double v)
	{
		rosic::ModBreakpoint::shapes shape;

		if (v > 0.0)
			return shape = rosic::ModBreakpoint::ANALOG;
		else if (v == 0.0)
			return shape = rosic::ModBreakpoint::LINEAR;
		else
			return shape = rosic::ModBreakpoint::GROWING;
	}

	double attack = 0.1;
	double decay = 0.5;
	double release = 1;
	double amp_scale = 1;
	double sustain_level = 1;

	double sustainLoopTime = 0.1;

	bool isLooping = true;

	bool isBypassed = false;
};

namespace rosicElanEdit
{

  /**

  This class specializes the very general BreakpointModulator into a kind of ADSR envelope.

  */

  class EnvelopeGenerator : public rosic::BreakpointModulator
  {

  public:
    //---------------------------------------------------------------------------------------------
    // construction/destruction:
    
    /** Constructor. */
    EnvelopeGenerator();

    //---------------------------------------------------------------------------------------------
    // parameter settings:

    /** Sets the point where the envelope starts (as raw value). */
    void setStartLevel(double newStart) { rosic::BreakpointModulator::setBreakpointLevel(0, newStart); }

    /** Sets the point where the envelope starts (in dB). */
    void setStartInDecibels(double newStart) { setStartLevel(RAPT::rsDB2amp(newStart)); }

    /** Sets the point where the envelope starts (in semitones). */
    void setStartInSemitones(double newStart) { setStartLevel(RAPT::rsPitchOffsetToFreqFactor(newStart)); }

    /** Sets the highest point of the envelope (as raw value). */
    void setPeakLevel(double newPeak) { setBreakpointLevel(1, newPeak); }

    /** Sets the highest point of the envelope (in dB). */
    void setPeakInDecibels(double newPeak) { setPeakLevel(RAPT::rsDB2amp(newPeak)); }

    /** Sets the highest point of the envelope (in semitones). */
    void setPeakInSemitones(double newPeak) { setPeakLevel(RAPT::rsPitchOffsetToFreqFactor(newPeak)); }

    double getSustainLevel() { return getBreakpointLevel(2); }

    /** Sets the sustain level (as raw value). */
    void setSustainLevel(double newSustain) 
    { 
      sustain_level = newSustain;
      setBreakpointLevel(2, sustain_level*amp_scale);
      setBreakpointLevel(3, getBreakpointLevel(2));
    }

    /** Sets the sustain level (in dB). */
    void setSustainInDecibels(double newSustain) { setSustainLevel(RAPT::rsDB2amp(newSustain)); }

    /** Sets the sustain level (in semitones). */
    void setSustainInSemitones(double newSustain) 
    { 
      setSustainLevel(RAPT::rsPitchOffsetToFreqFactor(newSustain));
    }

    /** Sets the end point of the envelope (as raw value). */
    void setEndLevel(double newEnd) { setBreakpointLevel(4, newEnd); }

    /** Sets the end point of the envelope (in dB). */
    void setEndInDecibels(double newEnd) { setEndLevel(RAPT::rsDB2amp(newEnd)); }

    /** Sets the end point of the envelope (in semitones). */
    void setEndInSemitones(double newEnd) { setEndLevel(RAPT::rsPitchOffsetToFreqFactor(newEnd)); }

    /** Sets the length of attack phase (in seconds). */
    void setAttack(double x) 
    { 
      attack = x;
      setBreakpointTime(1, attack);
      setBreakpointLevel(1, amp_scale);
    }

    /** Sets the hold time (in seconds). */
    void setHold(double newHoldTime);      

    /** Sets the length of decay phase (in seconds). */
    void setDecay(double x) 
    { 
      decay = x;
      setBreakpointTime(2, attack + decay);
      setBreakpointTime(3, attack + decay + decay2);
      setBreakpointLevel(2, sustain_level*amp_scale);
      setBreakpointLevel(3, sustain_level*amp_scale);
    }
 
    /** Sets the length of release phase (in seconds). */
    void setRelease(double x) 
    { 
      release = x;
      setBreakpointTime(4, attack + decay + decay2 + release);
      setBreakpointLevel(4, 0.0);
    }

    double attack = 0.1;
    double decay = 0.5;
		double decay2 = 1.0;
    double release = 1.0;
    double amp_scale = 1.0;
    double sustain_level = 1.0;

    void setAmplitudeScale(double x)
    {
      amp_scale = x;
      setBreakpointLevel(1, amp_scale);
      setBreakpointLevel(2, sustain_level*amp_scale);
      setBreakpointLevel(3, sustain_level*amp_scale);
    }

    void setShape(double x)
    {
      if (x > 0)
      {
        setBreakpointShape(1, rosic::ModBreakpoint::ANALOG);
        setBreakpointShape(2, rosic::ModBreakpoint::ANALOG);
        setBreakpointShape(4, rosic::ModBreakpoint::ANALOG);
        setBreakpointShapeAmount(1, std::min<double>(1, x));
        setBreakpointShapeAmount(2, x);
        //setBreakpointShapeAmount(4, min<double>(10, x));
        setBreakpointShapeAmount(4, x);
      }
      else
      {
        x=-x;
        setBreakpointShape(1, rosic::ModBreakpoint::ANALOG);
        setBreakpointShape(2, rosic::ModBreakpoint::GROWING);
        setBreakpointShape(4, rosic::ModBreakpoint::GROWING);
        setBreakpointShapeAmount(1, std::min<double>(1, x));
        setBreakpointShapeAmount(2, x);
        //setBreakpointShapeAmount(4, min<double>(10, x));
        setBreakpointShapeAmount(4, x);
      }
    }

    int id = -1;
    int mod_idx = -1;
    bool has_modulation_target = false;

    inline double getCurrentValue()
    {
      return previousOut;
    }

    enum class ResetMode { on_note, when_not_tied, when_silent } resetMode;
  };


} // end namespace rosic
