class AttackDecayFilter
{
	AttackDecayFilter()
    {
        reset();
        updateCoeffs();
    }

	/** Sets the attack time in samples, i.e. the number of samples it takes to reach the peak. */
	void setAttackSamples(double newAttack) { attackSamples = newAttack; coeffsDirty = true; }

	/** Sets the decay time constant in samples, i.e. the number of samples, it takes to decay to
	1/e for the more slowly decaying exponential. */
	void setDecaySamples(double newDecay) { decaySamples = newDecay; coeffsDirty = true; }


	//-----------------------------------------------------------------------------------------------
	/** \name Inquiry */

	/** Returns the gain of this filter at DC. This value can be useful to know when you want to 
	create an envelope with sustain - you may then feed the reciprocal of that value as constant 
	input into the filter. */
	double getGainAtDC() const { return s*(cd-ca) / (1+ca*cd-cd-ca); }

	/** Computes the reciprocal of the DC gain of this filter. This is the constant value, you want 
	to feed into the filter when you want to get a sustained output of 1. It's used for implementing 
	sustain in subclass rsAttackDecayEnvelope. */
	double getReciprocalGainAtDC() const { return (1+ca*cd-cd-ca) / (s*(cd-ca)); }


	//-----------------------------------------------------------------------------------------------
	/** \name Processing */

	double getSample(double v)
	{
		if(coeffsDirty)
			updateCoeffs();
		ya = v + ca * ya;
		yd = v + cd * yd;
		return s * (yd - ya);
	}

	/** Resets the internal state of both filters to zero. */
	void reset()
	{
		ya = yd = 0;
	}

	/** Updates our filter coefficients according to user parameters. */
	void updateCoeffs()
	{
		double tauAttack = 0;

		double attackSamples2 = min(0.99 * decaySamples, attackSamples + 1);
		// Why the 2nd +1? to avoid numerical problems when is goes down to zero? Then maybe 
		// using max would be better...or is there some offset of 1 sample that is being compesated?


		expDiffScalerAndTau2(decaySamples, attackSamples2, tauAttack, s);
		ca = exp(-1.0/tauAttack);			 // = exp(-alpha), pole radius
		cd = exp(-1.0/decaySamples);
		coeffsDirty = false;
	}
	// Data:
	double ca, cd;	// coefficients for attack and decay filters
	double ya, yd;	// state of attack and decay filter
	double s;
	bool coeffsDirty = true;	// flag to indicate that coeffs need to be re-computed
	//std::atomic<bool> coeffsDirty = true;	// flag to indicate that coeffs need to be re-computed
	double attackSamples = 20, decaySamples = 100; // sort of arbitrary
};

void expDiffScalerAndTau2(double tau1, double tp, double& tau2, double& scaler)
{
	if(tp >= tau1)
	{
		tau2   = tau1;
		scaler = 1.0;
		return;
	}
	double a1 = 1/tau1;
	double c = a1 * tp;
	double k  = findDecayScalerLess1(c);
	double a2 = k*a1;
	double hp = exp(-a1*tp) - exp(-a2*tp); // peak height

	tau2   = 1/a2;
	scaler = 1/hp;
}

double findDecayScalerLess1(double c)
{
  if(c <= 0.0 || c >= 1.0)
  {
    return 1.0;
  }

  // precomputations:
  double kp  = 1/c;                               // location of the the peak of g(k)
  double k   = 1 + 2*(kp-1);                      // initial guess for the zero of g(k)
  double eps =  2.22045e-16; // relative tolerance
  int    i   = 0;                            // iteration counter

  // Newton iteration:
  double g, gp;      // g(k), g'(k)
  double kOld = 2*k; // ensure to enter the loop
  while(abs(k-kOld) > k*eps && i < 1000)  // whoa! 1000? that seems way too high for production code!
  {
    kOld = k;
    g    = log(k) + c*(1-k); // g(k)
    gp   = 1/k - c;          // g'(k)
    k    = k - g/gp;         // Newton step
    i++;                     // count iteration
  }

  return k;
}


// maybe move into the same file where the modal filters are - it can be seen as a "modal" filter
// with zero frequency so it would fit in there

//=================================================================================================

/** An envelope generator based on rsAttackDecayFilter. It feeds the filter with a mix of a unit
impulse and a constant value, where the latter is responsible for a sustain phase */

// class attackDecayEnvelope : AttackDecayFilter
// {
//     int AccuFormula_none = 0;
//     int AccuFormula_exact = 1;
//     int AccuFormula_one_minus_yd = 2;

// 	//-----------------------------------------------------------------------------------------------
// 	/** \name Setup */

// 	/** Sets the sustain level. This is the value that is added to the filter's input as long a note
// 	is being held. */
// 	void setSustain(double v) 
//     {
//         sustain = v;
//     }

// 	void setAccumulationMode(int newMode)
// 	{
// 		accuFormula = newMode;
// 	}


// 	//-----------------------------------------------------------------------------------------------
// 	/** \name Inquiry */

// 	/** Returns the constant value that should be fed into the filter during the sustain phase. */
// 	double getSustainInput() const { return sustain * getReciprocalGainAtDC(); }


// 	//-----------------------------------------------------------------------------------------------
// 	/** \name Event Handling */

// 	//void trigger();	// maybe let it take a "strength" parameter?

// 	void noteOn(int key, int vel)
// 	{
// 		currentNote = key;

// 		double x = 1; // input to filter -	maybe should be scaled by vel?

// 		x = getAccuCompensatedImpulse();

// 		getSample(x);
// 	}

// 	void noteOff(int key, int vel)
// 	{
// 		currentNote = -1;
// 	}


// 	//-----------------------------------------------------------------------------------------------
// 	/** \name Audio Processing */

// 	double getSample()
// 	{
// 		if(currentNote != -1)	
// 			return getSample(getSustainInput());
// 		else									 
// 			return getSample(T(0));
// 	}

// 	void reset()
// 	{
// 		reset();
// 		currentNote = -1;
// 	}

// };

// 	/** Returns the desired input impulse height taking into account the current state of the filter.
// 	The goal is to compensate for the increasing height of the peaks when note-on events are received
// 	when the output has not yet decayed away. Without compensation, a quick succession of note-on 
// 	events will make the peak height grow. This function implements various formulas to be applied
// 	to the input impulse to counteract that effect. */
// 	double getAccuCompensatedImpulse()
// 	{
// 		switch(accuFormula)
// 		{
// 			//case 2: return 1 - yd;
// 			//case 1: return getExactAccuCompensation();
// 			default: return 1 - yd;
// 		}
// 	}

// 	int accuFormula = 0;

// 	double sustain = 0;
// 	int currentNote = -1; // -1 is code for "none"
// };