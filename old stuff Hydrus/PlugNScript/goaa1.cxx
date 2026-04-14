string name="ProtoSynth";
string description="First Synth";

array<string> inputParametersNames={"Amplitude", "Frequency", "Sandwich", "Reso", "Waveform", "Noise Reduction"};
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
double beanDip = 0.0;
double Sandwich = 0.05;
double Reso = 0.0;
double OscPos = 0.0;
double lastSample = 0.0;
double lastlastSample = 0.0;
double power = 0.0;

int noiseReduction = 1;
double noiseRedRat = 1.0 / (noiseReduction + 0.0);

int waveSize = 8191;
double waveSize1 = 1.0/(waveSize+0.0);
array<double> waveBasis(waveSize);
array<double> wavePercent(waveSize);
double flatRate = 1.0;

void processSample(array<double>& ioSample)
{
    // ** distortion before filter (oscillator shape)
    // ** adsr for amplitude and filter
    // ** lfo for amplitude and filter
    // ** delay/reverb for epicness
    // ** filters IIR

    // ** oscillator shape ideas
    // use parabol wave, not sine wave for wave creation
    // use wave morphing
    // use your saw/tri/sqare/parabol wave morph function
    // sin(x/5) -- use only a portion of a sine wave (parabol wave)
    // parabola
    // phase distortion
    // phase modulation
    // H Sync
    // vector sine distortion (a form of Phase Distoriton)
    // true frequency modulation (change oscillator speed per sample)
    // can also use phase distortion and phase modulation (just the phase offset is modulated)..
    // frequency modulation simply modulates the speed of the frequency

    // abs sigmoid, inv abs sigmoid
    // rnd (constant seed)
    // use smooth minimum (of absolute value and reapply sign after)
    
    // x*(1-abs(x))*4  -- parabolic wave (-1 to +1)
    // -abs(x)*2+1 -- triangle wave (-1 to +1)
    // 2*(2*(2*(2*(2*(2*(pi*x/(2^6))^2-1)^2-1)^2-1)^2-1)^2-1)^2-1  -- sine wave (-1 to 1)
    // phase distortion equation - 5*x/(abs(5*x)+1)*2/(5/3)  -- this makes it a saw (morph from a saw)
   
    // IDEA: dynamic scale , non-fixed scale  (either on multiples of 2 and 3, or on the first set of natural harmonics)

    sampleValue = 0;

    // ** noisy oscillator (for more analog)
    //double qrnd = ((rand()+rand()+rand()+rand()+rand())*0.4-1.0) * 0.00079;
    //OscPos = (OscPos + freqNormalized + qrnd) % 1.0; // move oscillator by frequency
    
    // ** clean oscillator
    // double qrnd = 0.0;
    // if (rand() > 0.99) qrnd = (rand() - 0.5) * 0.00079;
    // OscPos = (OscPos + freqNormalized + qrnd) % 1.0;      
     OscPos = (OscPos + freqNormalized) % 1.0;    
    
    // as you increase the iterations, the noise decreases
    // for (int i = 0; i < noiseReduction; ++i)
    // {
        rnd = ((rand()+rand()+rand()+rand()+rand())*0.4-1.0) * freqNormalized * 3.2;
        double sqr = (((OscPos + rnd) + 1000.0) % 1.0);

        // one possibility is using a premade buffer of random number of a sufficent length
        // then we can simply use this to select from linearly in a loop or at random..


        // another possibility is to sample at a random offset to regular sampling
        // and then interpolate what the next point would be for the a normal interval
        // sample before and after the next point.. and then interpolate the point..



        // ** cosine wave
        //sqr = cos(sqr*3.14159265*2);

        // ** square    
        // sqr = sqr - 0.5;
        // if (sqr<0) sqr = -1;
        // if (sqr>=0) sqr = +1;

        // ** saw
        //sqr = (sqr*2 -1);

        // ** half-cos saw
        sqr = cos(sqr*4 - 0.5);

        // ** saw bow
        // sqr = (pow(sqr*(-sqr +1),.1) *2-1) - 0.65

        // ** walter wave
        // double sqr2 = sqr * 2 % 1;
        // sqr = sqr - 0.5;
        // if (sqr<0) sqr = -0.25;
        // if (sqr>=0) sqr = +0.25;
        // sqr = sqr - ((pow(sqr2*(-sqr2 +1),.1) *2-1) - 0.65);

        // ** distortion (abs)
        // sqr = sqr * 7;
        // sqr = sqr / (abs(sqr) + 1);

        
        //sampleValue += sqr * amplitude;
    //}

    //sampleValue = sampleValue * noiseRedRat;

    
    int wavePos = int(OscPos * (waveSize+0.0));
    waveBasis[wavePos] = (waveBasis[wavePos] * (1.0- flatRate)) + (sqr * (flatRate));
    sampleValue = waveBasis[wavePos] * amplitude;

    flatRate = flatRate  * 0.999999;
    //flatRate = flatRate - 0.000001;
    if (flatRate < 0.001) flatRate = 0.001;

    // sampleValue = (lastlastSample +  lastSample + sampleValue) * 0.33333333333333333;
    // lastlastSample = lastSample;
    // lastSample = sampleValue;

    // ** Filter Feedback
    //sampleValue += followFilter * 0.4; // feedback, possibly use more delay
    //sampleValue += followFilter2 * 0.2;
    //sampleValue += followFilterB * 0.2;


    followFilterVelocity += ((sampleValue - followFilter) * Sandwich);
    followFilter = (followFilterVelocity + followFilter) * Reso;
    sampleValue = followFilter;

    followFilterVelocity2 += ((sampleValue - followFilter2) * Sandwich);
    followFilter2 = (followFilterVelocity2 + followFilter2) * Reso;
    
    //Bass Filter
    // followFilterVelocityB += ((followFilter2 - followFilterB) * 0.0005);
    // followFilterB = (followFilterVelocityB + followFilterB) * 0.99;
    // sampleValue = followFilter2 + (followFilterB * 0.4);
    
     sampleValue = followFilter2;

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
   freqNormalized=(inputParameters[1]*0.4)*(inputParameters[1]*0.4)*7;
   amplitude=inputParameters[0]*.5  * (1.0-freqNormalized);
   
   Sandwich=inputParameters[2]*inputParameters[2]*3;//4;
   
   Reso=inputParameters[3]*inputParameters[3]* 0.99;
   
   noiseReduction = floor(inputParameters[5] * 999.0 + 1.0);
   noiseRedRat = 1.0 / noiseReduction;
   flatRate = 1.0;
   //power = 1.414213562373095; // 1.5? 1.4?
}

int getTailSize()
{
    return -1;
}
