#include "library/Midi.hxx"
#include "library/Constants.hxx"

string name="AnalogWaveformProto";
string description="Analog Waveforms";

array<string> inputParametersNames={"Saw/Square"};
array<double> inputParameters(inputParametersNames.length,0);

//double PI = 3.14159265358979;
double tau = 3.14159265358979 * 2.0;
double k = 1.0;

double be = 0.0;

bool SawOrSqaure = false;

blit_saw_oscillator _blit_saw;

// note
class blit_saw_oscillator_note
{
    int ADSRStep;
    
    double adsrTime;
    double adsrLevel;
    // current time
    double t;

    // current value
    double value;

    // nyquist limit
    int n;

    // delta t
    double dt;

    // note number [0,127]
    int note_no;

    // velocity [0,1]
    double velocity;

    double integration;
    double dcB;
    // constructor
    blit_saw_oscillator_note()
    {
        adsrLevel = 0.0;
        adsrTime = 0.0;
        ADSRStep = 0;
        t = 0.0;
        value = 0.0;
        n = 0;
        dt = 0.0;
        note_no = 0;
        velocity = 0.0;
        integration = 0.0;
        dcB = 0.0;
    }
};

// oscillator class
class blit_saw_oscillator
{
    array<blit_saw_oscillator_note> _notes(16);
    uint _active_note_count;
    double _pitchbend;

    

    blit_saw_oscillator()
    {
        _active_note_count = 0;
        _pitchbend = 0.0;
    }

    void trigger(const MidiEvent& evt)
    {
        if( _active_note_count < _notes.length )
        {
            blit_saw_oscillator_note@ note = _notes[_active_note_count];

            note.note_no = MidiEventUtils::getNote(evt);
            note.velocity = MidiEventUtils::getNoteVelocity(evt)/127.0;
            note.value = 0.0;
            note.t = 0.5;
            note.adsrLevel = 0.0;
            note.ADSRStep = 0;

            //
            double freq = 440.0*(pow(2.0, (note.note_no + _pitchbend - 69.0) / 12.0));
            note.n = int(sampleRate / 2.0 / freq);
            note.dt = freq / sampleRate;

            ++_active_note_count;
        }
    }

    void update_pitchbend(const MidiEvent& evt)
    {
        _pitchbend = MidiEventUtils::getPitchWheelValue(evt)/4096.0;

        for (uint i = 0; i < _active_note_count; ++i)
        {
            blit_saw_oscillator_note@ note = _notes[i];

            double freq = 440.0*(pow(2.0, (note.note_no + _pitchbend - 69.0) / 12.0));
            note.n = int(sampleRate / 2.0 / freq);
            note.dt = freq / sampleRate;
        }
    }

    void release(const MidiEvent& evt)
    {
        int note_no = MidiEventUtils::getNote(evt);

        // //
        // uint idx;
        // for(idx = 0; idx < _active_note_count; ++idx)
        // {
        //     if( _notes[idx].note_no == note_no )break;
        // }

        // if( idx < _active_note_count )
        // {
        //     if( idx < _active_note_count - 1)
        //     {
        //         //    [1][ ][3][4][ ][ ]
        //         // -> [1][4][3][ ][ ][ ]
        //         _notes[idx] = _notes[_active_note_count - 1];
        //     }

        //     --_active_note_count;
        // }



        //
        uint idx;
        for(idx = 0; idx < _active_note_count; ++idx)
        {
            if( _notes[idx].note_no == note_no )
        	{
		        if( idx < _active_note_count )
		        {
		            if( idx < _active_note_count - 1)
		            {
		                //    [1][ ][3][4][ ][ ]
		                // -> [1][4][3][ ][ ][ ]
		                _notes[idx] = _notes[_active_note_count - 1];
		            }

		            --_active_note_count;
		        }

        	}
        }



    }

    //
    bool is_silent()
    {
        return _active_note_count == 0;
    }


    // Pure Harmonic Waveforms

    // Standard Saw (Removing DC Offset)
    // double bandlimited_impulse(double t, int n)
    // {
    //     //     if ( t < 1.0e-8 || 1.0-1.0e-8 < t )
    //     //     {
    //     //         return 2.0*(2*n+1);
    //     //     }

    //     //if (abs(t-0.5) < 0.0000001) return 1.0;
    //     //if (abs(t-1.0) < 0.00001) return 0.0;

    //     double part2 = (((sin(PI*4*t*(floor(n)*2+1)/4))/sin(t/4*4*PI)-1)/2);
    //     //return part2 /floor(n); // not for integration
    //      return part2; // for integration
    // }

    // Standard Square
    // double bandlimited_impulse(double t, int n)
    // {
    //     //     if ( t < 1.0e-8 || 1.0-1.0e-8 < t )
    //     //     {
    //     //         return 2.0*(2*n+1);
    //     //     }

    //     //if (abs(t-0.5) < 0.0000001) return 1.0;
    //     //if (abs(t-1.0) < 0.00001) return 0.0;
    //     double fn = floor(n*0.5);
    //     double part2 = (sin(4*PI*t*fn)/sin(2*PI*t));

    //     //return part2 / n; // for normal
    //     return part2 * 2.0; // for intergration
    // }

    
    // Bow Saw -- need harmonic version
    // double bandlimited_impulse(double t, int n)
    // {
    //     double part1= (((cos(PI*t)-cos(t*(4*floor(n)+2)*PI/2))/sin(t*PI)) + 2);
    //     // return part1 / 2.0 - 1.0; // for integration
    //     return (part1 / 2.0 - 1.0) / floor(n); // for normal
    // }

    // Bow Square
    // double bandlimited_impulse(double t, int n)
    // {
    //     double part1= (((1-cos(t*(floor(n/2)*16)*PI*0.25))/sin(t*PI*2)));
    //     return part1 / floor(n) * 2.0; // for normal
    //     // return part1; // for integration
    // }



    // Linear Harmonic Waveforms

    // Linear Saw -> Also if you integrate it you get a bow saw
    // double f1(double x, double n)
    // {
    //     return ((sin(x*n*PI))/sin(x*PI))/n;
    // }

    // double bandlimited_impulse(double t, int n)
    // {
    //     double part2 = (1.0-f1(t,floor(n*2+1))) / tan(t*PI);
    //     part2 = part2 / floor(n);
    //     if (part2 > 1.0 || part2 < -1.0) return 0.0;
    //     return part2; // no integration
    //     //return part2; // for integration
    // }

    // Linear Square  -> integrates into bow square
    // double f1(double x, double t)
    // {
    //     return ((sin(x*t*PI))/sin(x*PI))/t;
    // }

    // double bandlimited_impulse(double t, int n)
    // {
    //     // if (abs(t-0.5) < 0.00001) return 0.0;
    //     // if (abs(t-1.0) < 0.00001) return 0.0;
    //     double part2 = (1-f1(t*2,floor((n+0.0)/2.0)*2+1))/sin(t*PI*2);
    //     //if (part2  / floor(n+0.0) > 1.0) print("Partials" + n + "more " + t);
        
    //     part2 =  part2 / (floor(n+0.0) * 0.5); // no integration
    //     if (abs(part2) > 1.0) return 0.0; 
    //     return part2;

    //     // return part2; // for integration
    // }




    // Linear Falloff Saw Impulse -- integrates into slightly tri-like saw
    // double f1(double x, double t)
    // {
    //     double t1 = floor(t*2.0+2.0);
    //     if (int(floor(t)) % 2 == 0) return ( (1.0 + cos(x*t1*PI)) / cos(x*PI)) / t1;
    //     return ( (1.0 - cos(x*t1*PI)) / cos(x*PI)) / t1;
    // }

    // double bandlimited_impulse(double t, int n)
    // {
    //     double part2 = f1(t,n) / cos(t*PI);
    //     part2 = (part2/2.0-0.5) / floor(n) ; // not for integration
    //     if (abs(part2) > 1.0) return 0.0;
    //     return part2;
    //     //return part2/2.0-0.5; // for integration
    // }



    // Linear Falloff Square Impulse -- imntegrates into slightly tri-like saw
    // double f1(double x, double t)
    // {
    //     //double t1 = t*2.0+2.0;
    //     double t1 = t*2.0-2.0;
    //     if (int(floor(t)) % 2 == 0) return ((1+cos(x*t1*PI))/cos(x*PI))/t1;
    //     return ((1-cos(x*t1*PI))/cos(x*PI))/t1;
    // }

    // double bandlimited_impulse(double t, int n)
    // {
    //     double part2 = (f1(t*2,floor(n*0.5+1.0)))/cos(t*PI*2) * sin(PI*t*2);
    //     return part2 / floor(n) ; // not for integration
    //      // return part2; // for integration
    // }
    

    // Linear Falloff Square Impulse -- imntegrates into slightly tri-like saw
    // double f1(double x, double t)
    // {
    //     //double t1 = t*2.0+2.0;
    //     double t1 = t*2.0-2.0;
    //     if (int(floor(t)) % 2 == 0) return ((1+cos(x*t1*PI))/cos(x*PI))/t1;
    //     return ((1-cos(x*t1*PI))/cos(x*PI))/t1;
    // }



    // // Auto Filter Saw
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m);
    //     // sum(cos(m*x)*k^m, m = 1 .. n)
    //     double ret = (-pow(k,n+1)*cos((n+1)*x)-k*(-pow(k,n+1)*cos(x*n)+k-cos(x)))/(1-2*cos(x)*k+k*k);
    //     return ret;
    // }

    // Auto Filter Bow Saw
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m);
    //     // sum(sin(m*x)*k^m, m = 1 .. n)
    //     double ret = (sin(x)*k+pow(k,n+1)*(k*sin(x*n)-sin((n+1)*x)))/(1-2*cos(x)*k+k*k);
    //     return ret;
    // }

    // Auto Filter Square
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m * 0.5);
    //     // sum(cos((2*m-1)*x)*k^(2*m-1), m = 1 .. n)
    //     double ret = ((k*k*k*k-k*k)*cos(x)-pow(pow(k,n+1),2)*(cos(x*(2*n-1))*k*k-cos(x*(2*n+1))))/(-k*k*k*k*k+2*cos(2*x)*k*k*k-k);
    //     return ret;
    // }

    // Auto Filter Bow Square
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m * 0.5);
    //     // sum(sin((2*m-1)*x)*k^(2*m-1), m = 1 .. n)
    //     double ret = ((-k*k*k*k-k*k)*sin(x)-pow(pow(k,n+1),2)*(sin(x*(2*n-1))*k*k-sin(x*(2*n+1))))/(-k*k*k*k*k+2*cos(2*x)*k*k*k-k);
    //     return ret;
    // }

    // // Auto Filter Triharmonic
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m / 3.0);
    //     //sum(cos((3*m - 2)*x)*k^(3*m - 2), m = 1 .. n)
    //     double ret = (cos(x*(3*n-2))*pow(pow(k,n+1),3)*k*k*k-k*k*k*k*k*k*cos(2*x)+cos(x)*k*k*k-cos(x*(3*n+1))*pow(pow(k,n+1),3))/k*k/(1+k*k*k*k*k*k-2*cos(3*x)*k*k*k);
    //     return ret;
    // }

    // // Auto Filter TriharmonicBow
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m / 3.0);
    //     //sum(sin((3*m - 2)*x)*k^(3*m - 2), m = 1 .. n)
    //     //"((k^(n+1))^3*sin(x*(3*n-2))*k^3-(k^(n+1))^3*sin(x*(3*n+1))+k^3*(k^3*sin(2*x)+sin(x)))/k^2/(1+k^6-2*cos(3*x)*k^3)"
    //     //double ret = ((k^(n+1))^3*sin(x*(3*n-2))*k^3-(k^(n+1))^3*sin(x*(3*n+1))+k^3*(k^3*sin(2*x)+sin(x)))/k^2/(1+k^6-2*cos(3*x)*k^3);
    //     return ret;
    // }

    // attempt to make a composite filter.. not as good as the regular
    // double bandlimited_impulse(double u, int m)    
    // {
    //     double x = u * tau; // real phasor length
    //     double n = floor(m-1.0); // total partials altogether
    //     double zx = max(0.0,min(floor(be - 1.0), n)); // cutoff partials

    //     //double y = floor(n+1);
    //     double t = max(floor(be),1);
    //     double a = 1.0-be;
        
    //     double base = (-cos(x*zx)+1+cos(zx*x+x)-cos(x))/(-2+2*cos(x));

    //     //sum(k^(m + a)*cos(m*x), m = t .. n)="k^a*((-k*cos(x*(t-1))+cos(t*x))*k^t-k^(n+1)*(-k*cos(n*x)+cos((n+1)*x)))/(1-2*cos(x)*k+k^2)"
    //     double ret = pow(k,a)*((-k*cos(x*(t-1))+cos(t*x))*pow(k,t)-pow(k,n+1)*(-k*cos(n*x)+cos((n+1)*x)))/(1-2*cos(x)*k+k*k);

    //     if (floor(be) < 0) return ret;
    //     if (floor(be) > n) return base;
    //     return base + ret;
    //      // return ret;
    // }



    // analog saw
    double bandlimited_analog_sw(blit_saw_oscillator_note@ note)    
    {
        note.t =  (note.t + note.dt * 0.5 + (rand()-0.5) * 0.0001) % 2.0;
        double ou = (sin(note.t* PI * ((note.n+1.0) *4.0 -1.0)  ) / sin(note.t * PI)) / (note.n);
        double se = (pow (cos(note.t*PI*0.00125)*0.5+0.5, 5) * 0.031);
        note.integration = note.integration * (1.0 - se) + ou * se;
        note.dcB += 0.005 * (note.integration - note.dcB);
        note.integration =  note.integration * 0.94 + (note.integration - note.dcB) * (0.085 + (rand()-0.5) * 0.02)  ;
        return (note.integration - note.dcB) * 4.0;
    }


    // // // analog square 2
    double bandlimited_analog_sq(blit_saw_oscillator_note@ note)    
    {
        note.t =  (note.t + note.dt * 0.5 + (rand()-0.5) * 0.0001) % 2.0;
        double ou = (sin(note.t* PI * note.n * 4.0 ) / sin(note.t * PI)) / note.n;
        double se = (pow (cos(note.t*PI*0.00125)*0.5+0.5, 5) * 0.031);
        note.integration = note.integration * (1.0 - se) + ou * se;
        note.dcB += 0.005 * (note.integration - note.dcB);
        note.integration =  note.integration * 0.94 + (note.integration - note.dcB) * (0.085 + (rand()-0.5) * 0.02)  ;
        return (note.integration - note.dcB) * 4.0;
    }


    // // analog square
    // double bandlimited_analog(blit_saw_oscillator_note@ note)    
    // {
    //     note.t =  (note.t + note.dt * 0.5 + (rand()-0.5) * 0.0001) % 2.0;
    //     double ou = (sin(note.t* PI * note.n * 4.0 ) / sin(note.t * PI)) / note.n * 4.0;
    //     double se = (pow (cos(note.t*PI*0.00125)*0.5+0.5, 5) * 0.031);
    //     note.integration = note.integration * (1.0 - se) + ou * se;
    //     note.dcB += 0.005 * (note.integration - note.dcB);
    //     note.integration =  note.integration * 0.94 + (note.integration - note.dcB) * (0.085 + (rand()-0.5) * 0.02)  ;
    //     return (note.integration - note.dcB) * 2.0;
    // }


    double max(double a, double b)
    {
        if (a > b) return a;
        return b;
    }

    double min(double a, double b)
    {
        if (a < b) return a;
        return b;
    }

//=========================================================

    double timeInt = 1.0 / sampleRate;

    double process_sample()
    {
        double value = 0.0;
        for (uint i = 0; i < _active_note_count; ++i)
        {
            // render
            blit_saw_oscillator_note@ note = _notes[i];

            // update adsr            
            if (note.ADSRStep == 0)
            {
                note.adsrLevel = note.adsrLevel * (1.0 - Attack) + Attack;
                if (note.adsrLevel > 0.999999) 
                {
                    note.ADSRStep = 1;
                    print("Step" + note.ADSRStep);
                }
            }

            if (note.ADSRStep == 1)
            {
                note.adsrLevel = note.adsrLevel * (1.0 - Decay) + Sustain * Decay;
                if (abs(note.adsrLevel - Sustain) < 0.000001) 
                {
                        note.ADSRStep = 2;
                        print("Step" + note.ADSRStep);
                }
            }

            //k = 1.0- (1.0/(pow(note.adsrLevel * (1.0 - Relative),8)*10000.0+1.0)) + 0.00001; //1/(t+1)

            value += note.value * note.velocity;

            if (SawOrSqaure) note.value = bandlimited_analog_sq(note);
        	else note.value = bandlimited_analog_sw(note);

            // update
            // note.t += note.dt;
            // if (1.0 <= note.t)note.t -= 1.0;
            
            //note.t += note.dt * 0.9999; // added 0.9999 to help prevent errors
            //note.t = note.t - floor(note.t);
            
            // note.value = bandlimited_impulse(note.t, note.n);

            //note.value = note.value*0.998 + (bandlimited_impulse(note.t, note.n) )*note.dt; // for bow sqayre/saw
         
        }
        return value;
    }

    
};


double Attack = 0.0;
double Decay = 0.0;
double Sustain = 0.0;
double Release = 0.0;
double Relative = 0.0;

void updateInputParameters()
{
    // Relative = inputParameters[0];
    //k = inputParameters[2] * tau + tau;
    // be = pow(inputParameters[1],8) * 1000.0 + 1.0;
    //k = pow(inputParameters[0],0.020) * 0.95 + 0.05 ;

    //k = 1.0- (1.0/(pow(((inputParameters[0])/2+0.5),14)*10000.0+1.0)) + 0.00001; //1/(t+1)    
    // k = 1.0- (1.0/(pow(((1.0-inputParameters[0])/2+0.5),14)*10000.0+1.0)) + 0.00001; //1/(t+1)
    // k = 1.0- (1.0/(pow(inputParameters[0],8)*10000.0+1.0)) + 0.00001; //1/(t+1)
    
    //k = pow(1.0-inputParameters[0],3);
    // k = inputParameters[0] * 0.05 + .107; // .107 - .157
    // Attack = pow(1.0-inputParameters[1],8) * 0.9999 + 0.000001;
    // Decay = pow(1.0-inputParameters[2],8)  * 0.9999 + 0.000001;
    // Sustain = inputParameters[3];
    // Release = inputParameters[4]  * 0.9999 + 0.000001;
    
    if (inputParameters[0] > 0.5) SawOrSqaure = true;
    else SawOrSqaure = false;
}

void processBlock(BlockData& data)
{
    const MidiEvent@ event;
    if( 0 < data.inputMidiEvents.length )
    {
        @event = data.inputMidiEvents[0];
    }

    uint event_idx = 0;
    for(uint i=0; i<data.samplesToProcess; ++i)
    {
        while( @event != null && event.timeStamp <= double(i) )
        {
            MidiEventType evt_type = MidiEventUtils::getType(event);
            if( evt_type == kMidiNoteOn )
            {
                _blit_saw.trigger(event);
            }
            else if( evt_type == kMidiNoteOff )
            {
                _blit_saw.release(event);
            }
            else if( evt_type == kMidiPitchWheel )
            {
                _blit_saw.update_pitchbend(event);
            }

            ++event_idx;
            if( event_idx < data.inputMidiEvents.length )
            {
                @event = data.inputMidiEvents[event_idx];
            }
            else
            {
                @event = null;
            }
        }

        if( _blit_saw.is_silent() )continue;

        double value = _blit_saw.process_sample();
        for(uint ch = 0; ch < audioOutputsCount; ++ch)
        {
            data.samples[ch][i] = value;
        }
    }
}

int getTailSize()
{
    return -1;
}

