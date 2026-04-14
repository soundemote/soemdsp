// -- Direct Quasi-Bandlimited Oscillators (No-Integration) Saw/Square
// by Walter H. Hackett IV

#include "library/Midi.hxx"
#include "library/Constants.hxx"

string name="Direct Quasi-Bandlimited Oscillators";
string description=name;
array<double> inputParameters(1,0);
array<string> inputParametersUnits={""};
array<string> inputParametersNames={"Mode"};
array<int>    inputParametersSteps={,2};
array<double> inputParametersMax={1.0001};
array<string>  inputParametersEnums={"Saw;Square"};

blit_saw_oscillator _blit_saw;
const double tau = 3.14159265358979 * 2.0;

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
        uint idx;
        for(idx = 0; idx < _active_note_count; ++idx)
        {
            if( _notes[idx].note_no == note_no && idx < _active_note_count )
            {
                if( idx < _active_note_count - 1) _notes[idx] = _notes[_active_note_count - 1];
                --_active_note_count;
            }
        }
    }

    bool is_silent()
    {
        return _active_note_count == 0;
    }

    double bandlimited_impulse(double t, int n)
    {
        double x = t * tau;
        double m = floor(n + 1.0);
        if (m % 2.0 == 0.0) m = m + 1.0;
        double sinX = sin(x);

        // saw
        if (int(inputParameters[0]) == 0)
        {
            if (sinX < 0.0) return -pow(1.0-sin(x*m)/sinX/m,0.5) * cos(x);
            return pow(1.0-sin(x*m)/sinX/m,0.5) * cos(x);
        }

        // square
        if (sinX < 0.0) return -pow(1.0-sin(x*m)/sinX/m,0.5);
        return pow(1.0-sin(x*m)/sinX/m,0.5);
        
    }

    double process_sample()
    {
        double value = 0.0;
        for (uint i = 0; i < _active_note_count; ++i)
        {
            blit_saw_oscillator_note@ note = _notes[i];
            value += note.value * note.velocity;
            note.t += note.dt * 0.9999;
            note.t = note.t - floor(note.t);
            note.value = bandlimited_impulse(note.t, note.n);
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
