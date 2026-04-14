/*
 * blit_saw.cxx
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

#include "library/Midi.hxx"
#include "library/Constants.hxx"

string name="blit saw";
string description="BLIT-Based sawtooth wave synthesis";

array<string> inputParametersNames={"A","B","C","D"};
array<double> inputParameters(inputParametersNames.length,0);

// double PI = 3.14159265358979;
double tau = 3.14159265358979 * 2.0;
double k = 1.0;

double be = 0.0;

blit_saw_oscillator _blit_saw;

// note
class blit_saw_oscillator_note
{
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

    // constructor
    blit_saw_oscillator_note()
    {
        t = 0.0;
        value = 0.0;
        n = 0;
        dt = 0.0;
        note_no = 0;
        velocity = 0.0;
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

        //
        uint idx;
        for(idx = 0; idx < _active_note_count; ++idx)
        {
            if( _notes[idx].note_no == note_no )break;
        }

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
    double bandlimited_impulse(double t, int m)    
    {
        double x = t * tau;
        double n = floor(m);
        // sum(cos(m*x)*k^m, m = 1 .. n)
        double ret = (-pow(k,n+1)*cos((n+1)*x)-k*(-pow(k,n+1)*cos(x*n)+k-cos(x)))/(1-2*cos(x)*k+k*k);
        return ret;
    }

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

    // pulse width modulation as parameter
    double process_sample()
    {
        ham += 0.00003;
        ham = ham - floor(ham);
        
        k = 1.0- (1.0/(pow(0.5-ham*0.25,8)*10000.0+1.0)) + 0.00001;

        double value = 0.0;
        for (uint i = 0; i < _active_note_count; ++i)
        {
            // render
            blit_saw_oscillator_note@ note = _notes[i];
            value += note.value * note.velocity;

            // update
            // note.t += note.dt;
            // if (1.0 <= note.t)note.t -= 1.0;
            note.t += note.dt * 0.9999; // added 0.9999 to help prevent errors
            note.t = note.t - floor(note.t);
            // note.value = bandlimited_impulse(note.t, note.n);
            note.value = note.value*0.998 + (bandlimited_impulse(note.t, note.n) )*note.dt; // for bow sqayre/saw
         
        }
        return value;
    }

    double ham = 0.0;
};

void updateInputParameters()
{
    //k = inputParameters[2] * tau + tau;
    be = pow(inputParameters[1],8) * 1000.0 + 1.0;
    //k = pow(inputParameters[0],0.020) * 0.95 + 0.05 ;

    //k = 1.0- (1.0/(pow(((inputParameters[0])/2+0.5),14)*10000.0+1.0)) + 0.00001; //1/(t+1)    
    // k = 1.0- (1.0/(pow(((1.0-inputParameters[0])/2+0.5),14)*10000.0+1.0)) + 0.00001; //1/(t+1)
    // k = 1.0- (1.0/(pow(inputParameters[0],8)*10000.0+1.0)) + 0.00001; //1/(t+1)
    
    //k = pow(1.0-inputParameters[0],3);
    // k = inputParameters[0] * 0.05 + .107; // .107 - .157

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


    // Saw with Phase Rotation (after integration)
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m);
    //     double f = k;

    //     double ret = ((sin(n*x+f+x)-sin(n*x+f)+sin(f-x)-sin(f))/(-2+2*cos(x))) - sin(f) ;

    //     // return ret/n;
    //     return ret;
    // }


    // Walter Wave Saw (after integration)
    // double Pi = 3.14159265358979;
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor((m+1.0) * 0.5 - 1.0);
    //     double f = k;
        
        
    //     double c0 = x*(n+1);
    //     double c1 = cos(c0);
    //     double c2 = cos(x);
    //     double s1 = sin(c0);
    //     double s2 = sin(x);

    //     // walter wave
    //     double ret = (-c1*c1*c2-s1*(s2-1.0)*c1+c2)/s2;

    //     // for this to work, i think you need 2 equation depending on the number of partials..
    //     // for even select one, for odd, select the other.. and give the number of partials..


    //     //return (ret)/n;
    //      return -ret;
    // }


//sum(cos(m*x)*(1 - k^(y - m)), m = 0 .. n)
        //"k^y*k*(cos(x)-k)/(-1+2*cos(x)*k-k^2)*cos((n+1)*x)/(k^(n+1))+k^y*k*sin(x)/(-1+2*cos(x)*k-k^2)*sin((n+1)*x)/(k^(n+1))-1/2*cos((n+1)*x)-1/2*sin(x)/(cos(x)-1)*sin((n+1)*x)-k^y*k*(cos(x)-k)/(-1+2*cos(x)*k-k^2)+1/2"


 // Auto Filtered Saw (OLD 1)
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m);

    //     double expk = exp(k);
    //     double expn1 = exp((n + 1)*k);
    //     double cosx = cos(x);
    //     double cos1 = cos((n + 1)*x);
    //     //sum(cos(m*x)*exp(-k*m), m = 0 .. n) (normalized)
    //     double ret = expk*(cos1*cosx - expn1*cosx - cos1*expk 
    //         + expk*expn1 + sin(x)*sin((n + 1)*x)) /((-2*cosx*expk + 1 + expk*expk)*expn1);

    //     return 2.0*(ret-1);
    // }

    // Auto Filter Saw (OLD 2)
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m);
    //     double y = n;

    //     // sum(cos(m*x)*k^m, m = 0 .. n) =        
    //     double ret = (-pow(k,n+1)*cos((n+1)*x)+pow(k,n+2)*cos(x*n)-cos(x)*k+1)/(1-2*cos(x)*k+k*k);
    //     return ret-1.0;
    // }


    // double Pi = 3.14159265358979;
    // double bandlimited_impulse(double t, int m)    
    // {
    //     double x = t * tau;
    //     double n = floor(m);
    //     double f = k;
    //     // sum(cos(m*x+a)*(cos(b*m)+1), m = 0 .. n)=

    //     //"-1/2*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)*cos(b))/(cos(x)-cos(b))*cos((n+1)*x)*cos(b*(n+1))+1/2*cos(a)*sin(b)/(cos(x)-cos(b))*cos((n+1)*x)*sin(b*(n+1))+1/2*(sin(a)*cos(x)-sin(a)*cos(b)-cos(a)*sin(x))/(cos(x)-cos(b))*sin((n+1)*x)*cos(b*(n+1))-1/2*sin(a)*sin(b)/(cos(x)-cos(b))*sin((n+1)*x)*sin(b*(n+1))-1/2*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a))/(cos(x)-1)*cos((n+1)*x)+1/2*(sin(a)*cos(x)-cos(a)*sin(x)-sin(a))/(cos(x)-1)*sin((n+1)*x)+1/2*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)*cos(b))/(cos(x)-cos(b))+1/2*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a))/(cos(x)-1)"

    //     //"1/2*(((cos(x)-1)*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)*cos(b))*cos(b*(n+1))-cos(a)*sin(b)*(cos(x)-1)*sin(b*(n+1))-(-cos(x)+cos(b))*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)))*cos((n+1)*x)+((cos(x)-1)*(cos(a)*sin(x)+sin(a)*cos(b)-sin(a)*cos(x))*cos(b*(n+1))+sin(a)*sin(b)*(cos(x)-1)*sin(b*(n+1))-(-cos(x)+cos(b))*(cos(a)*sin(x)-sin(a)*cos(x)+sin(a)))*sin((n+1)*x)-2*cos(x)^2*cos(a)+((2*cos(b)+2)*cos(a)-2*sin(a)*sin(x))*cos(x)-2*cos(a)*cos(b)+sin(a)*sin(x)*(cos(b)+1))/(-cos(x)+cos(b))/(cos(x)-1)"
    //     double a = k; //double a = k;
    //     double b = be;
    //     double c = 0;


    //     double ret = 0.5*(((cos(x)-1)*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)*cos(b))*cos(b*(n+1))-cos(a)*sin(b)*(cos(x)-1)*sin(b*(n+1))-(-cos(x)+cos(b))*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)))*cos((n+1)*x)+((cos(x)-1)*(cos(a)*sin(x)+sin(a)*cos(b)-sin(a)*cos(x))*cos(b*(n+1))+sin(a)*sin(b)*(cos(x)-1)*sin(b*(n+1))-(-cos(x)+cos(b))*(cos(a)*sin(x)-sin(a)*cos(x)+sin(a)))*sin((n+1)*x)-2*cos(x)*cos(x)*cos(a)+((2*cos(b)+2)*cos(a)-2*sin(a)*sin(x))*cos(x)-2*cos(a)*cos(b)+sin(a)*sin(x)*(cos(b)+1))/(-cos(x)+cos(b))/(cos(x)-1);
        

    //     //double ret = 0.5*(-cos(a)*cos(x)*cos(x)*cos(c)+cos(x)*cos(x)*cos(a)*cos(x*(n+1))-cos(x)*cos(x)*sin(a)*sin(x*(n+1))+cos(x)*cos(a)*cos(b)-cos(x)*cos(a)*cos(x*(n+1))-cos(x)*sin(a)*sin(x)+cos(x)*sin(a)*sin(x*(n+1))-cos(a)*cos(c)*cos(b)-cos(a)*sin(b)*sin(c)+cos(a)*cos(b)*cos(x*(n+1))+sin(a)*sin(x)*cos(b)-cos(b)*sin(a)*sin(x*(n+1))+cos(x)*cos(a)*cos(c)*cos(b)+cos(x)*cos(a)*sin(b)*sin(c)-cos(x)*cos(a)*cos(b)*cos(x*(n+1))+cos(x)*cos(a)*sin(x)*sin(x*(n+1))-cos(x)*cos(c)*sin(a)*sin(x)+cos(x)*cos(b)*sin(a)*sin(x*(n+1))+cos(x)*sin(a)*sin(x)*cos(x*(n+1))-cos(a)*cos(b)*sin(x)*sin(x*(n+1))-cos(b)*sin(a)*sin(x)*cos(x*(n+1))+cos(a)*cos(c)*cos(x)+sin(a)*cos(c)*sin(x)+cos(x)*cos(a)+cos(x)*cos(x)*cos(a)*cos(c)*cos(b*(n+1))*cos(x*(n+1))-cos(x)*cos(x)*cos(a)*sin(c)*sin(b*(n+1))*cos(x*(n+1))-cos(x)*cos(x)*cos(c)*sin(a)*cos(b*(n+1))*sin(x*(n+1))+cos(x)*cos(x)*sin(c)*sin(a)*sin(b*(n+1))*sin(x*(n+1))-cos(x)*cos(a)*cos(c)*cos(b*(n+1))*cos(x*(n+1))+cos(x)*cos(a)*sin(c)*sin(b*(n+1))*cos(x*(n+1))+cos(x)*cos(c)*sin(a)*cos(b*(n+1))*sin(x*(n+1))-cos(x)*sin(c)*sin(a)*sin(b*(n+1))*sin(x*(n+1))+cos(a)*cos(c)*sin(b)*sin(b*(n+1))*cos(x*(n+1))+cos(a)*cos(c)*cos(b)*cos(b*(n+1))*cos(x*(n+1))-cos(a)*cos(c)*sin(x)*cos(b*(n+1))*sin(x*(n+1))+cos(a)*sin(b)*sin(c)*cos(b*(n+1))*cos(x*(n+1))-cos(a)*sin(c)*cos(b)*sin(b*(n+1))*cos(x*(n+1))+cos(a)*sin(c)*sin(x)*sin(b*(n+1))*sin(x*(n+1))-cos(c)*sin(b)*sin(a)*sin(b*(n+1))*sin(x*(n+1))-cos(c)*cos(b)*sin(a)*cos(b*(n+1))*sin(x*(n+1))-cos(c)*sin(a)*sin(x)*cos(b*(n+1))*cos(x*(n+1))-sin(b)*sin(c)*sin(a)*cos(b*(n+1))*sin(x*(n+1))+sin(c)*cos(b)*sin(a)*sin(b*(n+1))*sin(x*(n+1))+sin(c)*sin(a)*sin(x)*sin(b*(n+1))*cos(x*(n+1))-cos(a)*cos(x)*cos(x)-cos(a)*cos(b)-cos(x)*cos(a)*cos(c)*sin(b)*sin(b*(n+1))*cos(x*(n+1))-cos(x)*cos(a)*cos(c)*cos(b)*cos(b*(n+1))*cos(x*(n+1))+cos(x)*cos(a)*cos(c)*sin(x)*cos(b*(n+1))*sin(x*(n+1))-cos(x)*cos(a)*sin(b)*sin(c)*cos(b*(n+1))*cos(x*(n+1))+cos(x)*cos(a)*sin(c)*cos(b)*sin(b*(n+1))*cos(x*(n+1))-cos(x)*cos(a)*sin(c)*sin(x)*sin(b*(n+1))*sin(x*(n+1))+cos(x)*cos(c)*sin(b)*sin(a)*sin(b*(n+1))*sin(x*(n+1))+cos(x)*cos(c)*cos(b)*sin(a)*cos(b*(n+1))*sin(x*(n+1))+cos(x)*cos(c)*sin(a)*sin(x)*cos(b*(n+1))*cos(x*(n+1))+cos(x)*sin(b)*sin(c)*sin(a)*cos(b*(n+1))*sin(x*(n+1))-cos(x)*sin(c)*cos(b)*sin(a)*sin(b*(n+1))*sin(x*(n+1))-cos(x)*sin(c)*sin(a)*sin(x)*sin(b*(n+1))*cos(x*(n+1)))/(-cos(x)+cos(b))/(cos(x)-1);

    //     // double ret = 0.5*(((cos(x)-1)*(cos(a)*cos(c)*cos(x)+(-sin(b)*sin(c)-cos(b)*cos(c))*cos(a)+sin(a)*cos(c)*sin(x))*cos(b*(n+1))
    //     //     -(cos(x)*cos(a)*sin(c)+sin(x)*sin(c)*sin(a)+cos(a)*(sin(b)*cos(c)-cos(b)*sin(c)))*(cos(x)-1)*sin(b*(n+1))
    //     //     -(-cos(x)+cos(b))*(sin(a)*sin(x)+cos(a)*cos(x)-cos(a)))*cos(x*(n+1))+((cos(x)-1)
    //     //     *(-cos(x)*cos(c)*sin(a)+(sin(b)*sin(c)+cos(b)*cos(c))*sin(a)+sin(x)*cos(a)*cos(c))
    //     //     *cos(b*(n+1))+(cos(x)*sin(c)*sin(a)+(sin(b)*cos(c)-cos(b)*sin(c))*sin(a)-sin(x)*cos(a)
    //     //     *sin(c))*(cos(x)-1)*sin(b*(n+1))+(-cos(x)+cos(b))*(sin(a)*cos(x)-cos(a)*sin(x)-sin(a)))
    //     //     *sin(x*(n+1))-cos(a)*(cos(c)+1)*cos(x)*cos(x)+(((cos(b)+1)*cos(c)+sin(b)*sin(c)+cos(b)+1)
    //     //     *cos(a)-sin(x)*sin(a)*(cos(c)+1))*cos(x)+(-sin(b)*sin(c)-cos(b)*cos(c)-cos(b))*cos(a)
    //     //     +sin(x)*sin(a)*(cos(b)+cos(c)))/(-cos(x)+cos(b))/(cos(x)-1);
        //double ret = -0.5*(sin(1)-sin(x))/(cos(x)-cos(1))*cos((n+1)*x)*cos(n+1)-0.5*cos((n+1)*x)*sin(n+1)-0.5*sin((n+1)*x)*cos(n+1)+0.5*(sin(1)-sin(x))/(cos(x)-cos(1))*sin((n+1)*x)*sin(n+1)+0.5*(sin(1)-sin(x))/(cos(x)-cos(1));        
        //double ret = ((-1.0)/sin(x))*cos(x*(n+1.0))*cos(x*(n+1.0))+(1.0/sin(x));

        
    //     //return (ret)/n;
    //     //return ret  * 0.5 - cos(a) + sin(c ) *  0.5;
    //     //return ret  * 0.5 - cos(a) - (sin(-c * 0.5) * 0.5);
    //     //return (ret + cos(c-Pi)+1.0) - 2.0;
    //     return ret - cos(a) * 2.0 + (cos(c-Pi) + 1.0) ;

    // }
