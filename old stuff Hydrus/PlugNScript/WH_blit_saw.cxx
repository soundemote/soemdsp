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
    double f1(double x, double t)
    {
        //double t1 = t*2.0+2.0;
        double t1 = t*2.0-2.0;
        if (int(floor(t)) % 2 == 0) return ((1+cos(x*t1*PI))/cos(x*PI))/t1;
        return ((1-cos(x*t1*PI))/cos(x*PI))/t1;
    }

    double bandlimited_impulse(double t, int n)
    {
        double part2 = (f1(t*2,floor(n*0.5+1.0)))/cos(t*PI*2) * sin(PI*t*2);
        return part2 / floor(n) ; // not for integration
         // return part2; // for integration
    }
    


    //
    double process_sample()
    {
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
            note.value = bandlimited_impulse(note.t, note.n);
            // note.value = note.value*0.998 + (bandlimited_impulse(note.t, note.n) )*note.dt; // for bow sqayre/saw
         
        }
        return value;
    }
};

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
