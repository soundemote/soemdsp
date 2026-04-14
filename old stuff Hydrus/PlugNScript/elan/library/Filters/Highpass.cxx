class Highpass 
{
    void setSampleRate(double v)
    {
        sampleRate = v;
        update();
    }

    void setFrequency(double v)
    {
  		frequency = v;
  		update();
    }

    double getSample(double v)
    {
	    y1 = b0*v + b1*x1 + a1*y1;
	    x1 = v;
	    return y1;
    }

	void reset()
	{
		x1 = 0;
		y1 = 0;
	}

    protected void update()
    {
        freqToOmega = TAU/sampleRate;
        w = freqToOmega*frequency;

        a1 = exp(-w);
		b0 =  0.5*(1 + a1);
		b1 = -b0;
    }

    double frequency = 0;
    double sampleRate = 44100;
    int mode = 0;

    protected double freqToOmega = 0;

	// buffering
	protected double x1 = 0;
	protected double y1 = 0;

	// filter coefficients:
	protected double b0 = 1;
	protected double b1 = 0;
	protected double a1 = 0;
	protected double w  = 0;    
}