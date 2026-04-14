class NoteTracker
{
    void addNote(int note, double amplitude)
    {
        int noteFind = currentNotes.find(note);

        notes[note] = amplitude;

        if (noteFind == -1)        
        {    
            currentNotes.insertLast(note);
            currentNotes.sortAsc();
        }
    }

    void removeNote(int note)
    {
        int noteFind = currentNotes.find(note);

        notes[note] = 0;

        if (noteFind != -1)
        {
            currentNotes.removeAt(noteFind);
            currentNotes.sortAsc();            
        }
    }

    double getAmplitudeForNote(int note)
    {
        return notes[note];
    }

    // provide a number between -128 and + 128
    int getNote(double v)
    {
        if (getNumNotes() == 0)
            return 0;

        double octave = v / 12.0;

        double frac = fraction(octave);

        int indexToPlay = int(map(frac, 0, getNumNotes() - 1));

        int i = indexToPlay % getNumNotes();

        int semitoneToPlay = noteTracker.currentNotes[i];        

        return int(clamp(roundToInt(semitoneToPlay + floor(octave) * 12), 0, 127));
    }

    int getNumNotes()
    {
        return currentNotes.length;
    }

    array<double> notes(128);
    array<int> currentNotes;
}
