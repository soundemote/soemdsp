/** \file
*   Simple sawtooth generator.
*   Charles Verron - 2016 - Public domain
*/

string name="Simple sawtooth";
string description="Simple sawtooth generator";

array<string> inputParametersNames={"Amplitude", "Frequency", "Sandwich", "Reso"};
array<double> inputParameters(inputParametersNames.length);
array<double> inputParametersDefault={.5, .5};

double amplitude, freqNormalized;
double sampleValue = 0;
double minFreqHz = 50;
double maxFreqHz = 96000;
double freqRange = maxFreqHz - minFreqHz;
double t = 0;
double rnd = 0.0;
double followFilter = 0.0;
double followFilterVelocity = 0.0;
double followFilter2 = 0.0;
double followFilterVelocity2 = 0.0;
double followFilterB = 0.0;
double followFilterVelocityB = 0.0;

double Sandwich = 0.05;
double Reso = 0.0;
double OscPos = 0.0;
double lastSample = 0.0;
double lastlastSample = 0.0;
double feedback = 0.0;

int noiseReduction = 30;
double noiseRedRat = 1.0 / (noiseReduction + 0.0) ;
void processSample(array<double>& ioSample)
{
    // how about a follow based curve... where it's maximum rate of change
    // cannot be higher than a certain number over time...
    // is gradually moves towards that point is all...
    // is it percentage motion or is it linear motion?
    // velocity (momentum) and direction changes require slow down...
    // max velocity
    // max change of velocity
    // different up vs down velocities to make a saw
    // possibly can be used to low pass before sample on synths
    // using crossfade as a mechanism to create traction
    // or using the oscillation to create a gibbs effect

    // change of direction to be slowed or sped up
    // upward motion and downward motion seperate
    // use 0 to 1 with moulus for rotation of formula-based oscillator
    // can also use phase distortion and phase modulation (just the phase offset is modulated)..
    // frequency modulation simply modulates the speed of the frequency


    // oscillator ideas
    // use parabol wave, not sine wave for wave creation
    // use wave morphing
    // use your saw/tri/sqare/parabol wave morph function
    // sin(x/5) -- use only a portion of a sine wave (parabol wave)
    // parabola
    //floor(x/1)*1 - 1 * x + 1 * sin(x * 8)* 5
    //(x^5)/(5^4)
    //5*sin((x/1.59155)+1.59155) .
    //1 * x + 1 * sin(x * 8)
    //5 * tanh(x/PI*9) -- use your abs sigmoid instead
    // floor(x) + sin(x) 
    // rnd (constant seed)
    //5 * cos(x/PI* 9) 
    //(4.17*log(6-x))-5 + floor(x/5)*5 + 2
    // use smooth minimum
    // tan(cos(x))* sin(x*0.6)* 3
    //x^sign(x)
    // 0.08441654*sinh(x/(pi/3)) -- use inverse abs sigmoid instead
    // tan(x) -- use inverse abs sigmoid instead
    //abs(sin(x/2))*10 -5
    //5*asin(x)
    //floor(x) / sin(x) * 0.2
    //(x^13)/(5^12)
    //2* sin(x/PI)* floor(x) - 5
    //(cosh(x)/sin(-x) - 5)/100
    //(cosh(x)/sin(-x) - 5)/100
    //(sin(x)*5) + (sin(x* 6)*0.5) + (sin(x* 11)*0.2)
    //(x^5)/(5^4)+ floor(x/5)*2.5
    //(x^4)*0.04   /5 + floor(x/1)*1
    //((abs(x)^0.5) * 2) + (4 * acos(x/PI))-10
    // x^sin(x) * 5 -5

    // -> very close to saw phase rotation wave and wlterwave
    // (((w*(-w+1))^.1)*2-1)-.65

    // dynamic scale

    // is is possible to use the follower filter as a sinc function..
    // the follow would have to be never higher than a specific frequency
    // which means that it much never turn around faster than a specific rate
    // and it must be a perfect cosine wave in in following
    // esseentially a cosine wave with a following aspect to the wave

    sampleValue = 0;

    // noisy oscillator
    double qrnd = ((rand()+rand())*0.5-0.5) * 0.002;
    OscPos = (OscPos + freqNormalized + qrnd) % 1.0; // move oscillator by frequency

    // antialias ideas -- lP in cereal, following cosine (where peak of next follows sample setmight need to use low frequency to makeup for changes in the amp)
    

    // as you increase the iterations, the noise decreases, but the gaussian sampling shape must also change too to compensate
    // thousand of iterations are needed to make it truly lower
    for (int i = 0; i < noiseReduction; ++i)
    {
        rnd = ((rand()+rand()+rand()) - 1.5) * freqNormalized * 1.5;
        //rnd = ((rand()+rand()+rand()) - 1.5) * freqNormalized * (2+(rand()-0.5));
        //rnd = ((rand()+rand()+rand()) - 1.5) * freqNormalized * 4;
        double sqr = (((OscPos + rnd) + 1000.0) % 1.0);

        // ** square    
         // sqr = sqr - 0.5;
         // if (sqr<0) sqr = -1;
         // if (sqr>=0) sqr = +1;

        // ** saw
        sqr = (sqr*2 -1);

        // ** half-cos saw
        //sqr = cos(sqr*4 - 0.5);

        // cosine wave
        // sqr = cos(sqr*3.14159265*2);
        // distortion (abs)
        // sqr = sqr * 7;
        // sqr = sqr / (abs(sqr) + 1);

        // try using the median (requires a sort)
        sampleValue += sqr * amplitude;
    }

    sampleValue = sampleValue * noiseRedRat;
    // sampleValue = (lastlastSample +  lastSample + sampleValue) * 0.33333333333333333;
    // lastlastSample = lastSample;
    // lastSample = sampleValue;

    //sampleValue += followFilter * 0.4; // feedback, possibly use more delay
    //sampleValue += followFilter2 * 0.2;
    //sampleValue += followFilterB * 0.2;

    followFilterVelocity += ((sampleValue - followFilter) * Sandwich);
    followFilter = (followFilterVelocity + followFilter) * Reso;
    sampleValue = followFilter;

    followFilterVelocity2 += ((sampleValue - followFilter2) * Sandwich);
    followFilter2 = (followFilterVelocity2 + followFilter2) * Reso;
    
    //Bass Filter
    followFilterVelocityB += ((followFilter2 - followFilterB) * 0.0005);
    followFilterB = (followFilterVelocityB + followFilterB) * 0.99;
    sampleValue = followFilter2 + (followFilterB * 0.4);
    
    // sampleValue = followFilter2;

    if (Sandwich == 0.0) { followFilterVelocity *= 0.99; followFilterVelocity2 *= 0.99; }
    

    for(uint channel=0;channel<audioOutputsCount;channel++)
    {
        ioSample[channel]=sampleValue;
    }
}

void updateInputParameters()
{
   amplitude=inputParameters[0]*.5; 
   //freqNormalized=(inputParameters[1]*freqRange + minFreqHz)/sampleRate;
   freqNormalized=(inputParameters[1]*0.4)*(inputParameters[1]*0.4)*4;
   amplitude=inputParameters[0]*.5  * (1.0-freqNormalized);
   Sandwich=inputParameters[2]*inputParameters[2]*3;//4;
   Reso=inputParameters[3]*inputParameters[3]* 0.99;
   feedback = inputParameters[2];
}

int getTailSize()
{
    return -1;
}
