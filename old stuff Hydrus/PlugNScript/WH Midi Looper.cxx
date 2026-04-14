//=================================================
// Midi Looper
//=================================================

#include "library/Midi.hxx"

//=================================================
// Primary Variables
//=================================================

string name="Walter Midi Looper";
string description=name;
array<string> inputParametersNames={"State"};
array<double> inputParameters(inputParametersNames.length,0);

BlockData@ bl;

// Time Quantization Level 
const int Quantize = 16; // Generally a power of 2(32, 16, 8, 4), but can multiples of 3 as well (6,12)
const int TimeMult = int(Quantize / 4);
// ** possiblity of longer bar lengths and different quantizations (1 bar,2 bar,4 bars), Quantizations 32,16,8

bool EditMode = false;
int LevelSelected = 0; // current level selected

enum LevelSelect // available levels
{
    kLvlNotes,
    kLvlPhrase,
    kLvlSection,
    kLvlSong,
    kLvlAlbum
};

double Time;
int InternalSampleCount = 0;
int lastQuater = 0;

//=================================================
// Notes Level
//=================================================
int SelectedKeyNote = 0;  // Current Key Selected
int NoteBankSelected = 0; // Curent Bank Selected
NotePerKey theMidiLoop;

// add something to hold 'type' for 1 bar, 2 bar, 4 bar, or all 4 bar, but first bar or 2 bars repeating
class NotePart { array<MidiEvent> Midi; } // Array of Midi Events
class NoteQuant { array<NotePart> Part(Quantize); } // 1 bar divided into 16 parts
class NotePerKey { array<NoteQuant> Key(128); } // Assigned to a keyboard key (0-127)
class NoteBank { array<NotePerKey> Banks; } // unlimited number of banks (array for now, later linked list so open in 2 directions (freedom))

//=================================================
// Phrase Level
//=================================================
// next level is 4 bars long, and it consists of 4 parts of the midi loops above, compoundable
// each assignable to a key 0-127
// 4 bars long, but should it be quantized into 16ths of a bar? (64 PhraseKeys).. this would also require a metronome
// phrase is on or off..
// should notes on be turned off at the end of a phrase?

int SelectedKeyPhrase = 0;  // Current Key Selected
int PhraseBankSelected = 0; // Curent Bank Selected
PhrasePerKey thePhraseLoop;
class PhraseKey { array<int> Keys; } // each phrase to play at a moment one time
class PhraseQuant { array<PhraseKey> Part(4); } // 4 bars long in loop
class PhrasePerKey { array<PhraseQuant> Key(128); } // assignable to key
class PhraseBanks { array<PhrasePerKey> Banks; } // banks to add as many as you want

//=================================================
// Section Level
//=================================================

int SelectedKeySection = 0;     // Current Key Selected
int SectionBankSelected = 0;    // Curent Bank Selected
SectionPerKey theSectionLoop;
class SectionKey { array<int> Keys; } // each phrase to play at a moment one time
class SectionQuant { array<SectionKey> Part(4); } // 16 bars long in loop, Quantitzed to 4 bars
class SectionPerKey { array<SectionQuant> Key(128); } // assignable to key
class SectionBanks { array<SectionPerKey> Banks; } // banks to add as many as you want

//=================================================
// Song Level
//=================================================
// Non-Compandable
// this level is of infinite length, but quantitized into 16 bars
// simply plays all sections in order, in a loop (one after the next)
// each assignable to a key

int SelectedKeySong = 0;     // Current Key Selected
int SongBankSelected = 0;    // Curent Bank Selected
SongPerKey theSongLoop;
class SongKey { array<int> Keys; } // Open numbars long in loop, Plays Sequentially
class SongPerKey { array<SongKey> Key(128); } // assignable to key
class SongBanks { array<SongPerKey> Banks; } // banks to add as many as you want

//=================================================
// Album Level
//=================================================
// Non-Compandable
// this level is of infinite items, but quantitized into 64 bars, 
// simply plays all Songs in order, in a loop (one after the next)
// each assignable to a key

int SelectedKeyAlbum = 0;    // Current Key Selected
int AlbumBankSelected = 0;   // Curent Bank Selected
AlbumPerKey theAlbumLoop;
class AlbumKey { array<int> Keys; } // Open numbars long in loop, Plays Sequentially
class AlbumPerKey { array<AlbumKey> Key(128); } // assignable to key
class AlbumBanks { array<AlbumPerKey> Banks; } // banks to add as many as you want

//=================================================
// Sheet
//=================================================
// just saves a loads everything below, so it doesnt eat all memory
// could save entire sheet as a midi file
// export all note banks first, then all phrase banks, etc etc, by the keys only


//=================================================
// Main Function
//=================================================

void processBlock(BlockData& data)
{
    if (!Enabled) return;
    if (!FreeRun && !data.transport.isPlaying) return;

    @bl = @data;

    // ========================================
    // Input - Handle the Notes
    // ========================================
    if (data.transport.isPlaying) 
    {
        InternalSampleCount = int(bl.transport.positionInSamples);
    }

        if (bl.inputMidiEvents.length > 0)
        for(uint i=0;i<bl.inputMidiEvents.length;i++)
        {

            // Edit Mode Toggle Handle
            if (MidiEventUtils::getType(bl.inputMidiEvents[i]) == kMidiControlChange)
            {
                if (MidiEventUtils::getCCNumber(bl.inputMidiEvents[i]) == 113)
                {
                    if (MidiEventUtils::getCCValue(bl.inputMidiEvents[i]) == 127)
                    {
                        // Edit Mode On/Off
                        EditMode = !EditMode;

                        // Clear Loop by Selected Key
                        if (EditMode) 
                        {
                            if (LevelSelected == kLvlNotes)
                                for (int k = 0; k < Quantize; ++k) 
                                    theMidiLoop.Key[SelectedKeyNote].Part[k].Midi.resize(0);

                            if (LevelSelected == kLvlPhrase)
                                for (uint k = 0; k < thePhraseLoop.Key[SelectedKeyPhrase].Part.length; ++k)
                                    thePhraseLoop.Key[SelectedKeyPhrase].Part[k].Keys.resize(0);

                            if (LevelSelected == kLvlSection)
                                for (uint k = 0; k < theSectionLoop.Key[SelectedKeySection].Part.length; ++k)
                                    theSectionLoop.Key[SelectedKeySection].Part[k].Keys.resize(0);

                            if (LevelSelected == kLvlSong)
                            {
                                for (uint k = 0; k < theSongLoop.Key[SelectedKeySection].Keys.length; ++k)
                                    theSongLoop.Key[SelectedKeySong].Keys.resize(0);
                            }

                            if (LevelSelected == kLvlAlbum)
                            {
                                for (uint k = 0; k < theAlbumLoop.Key[SelectedKeySection].Keys.length; ++k)
                                    theAlbumLoop.Key[SelectedKeyAlbum].Keys.resize(0);
                            }

                            AllNotesOff();  // Turn all Notes Off After Clearing Loop
                        }

                        print("Edit Mode: " + EditMode);
                        break;
                    }
                }

                // Level change
                if (MidiEventUtils::getCCNumber(bl.inputMidiEvents[i]) == 115)
                {
                    if (MidiEventUtils::getCCValue(bl.inputMidiEvents[i]) == 127)
                    {
                        if (!EditMode)
                        {
                             // perhaps a number of beeps out to show the level..
                            // Level Change (Rotating initially)
                            LevelSelected = (LevelSelected + 1) % (kLvlAlbum + 1);
                            AllNotesOff();
                            if (LevelSelected == kLvlNotes) print("Level Selected: Notes");
                            if (LevelSelected == kLvlPhrase) print("Level Selected: Phrase");
                            if (LevelSelected == kLvlSection) print("Level Selected: Section");
                            if (LevelSelected == kLvlSong) print("Level Selected: Song");
                            if (LevelSelected == kLvlAlbum) print("Level Selected: Album");
                        }
                    }
                }
            }

            MidiEventType _ty = MidiEventUtils::getType(bl.inputMidiEvents[i]);

            // Edit Mode 
            if (EditMode)
            {
                //double Time = samplesToQuarterNotes(InternalSampleCount + bl.inputMidiEvents[i].timeStamp,bl.transport.bpm) + 0.5;
                double Time = samplesToQuarterNotes(InternalSampleCount + bl.inputMidiEvents[i].timeStamp,bl.transport.bpm) + 0.25;

                if (LevelSelected == kLvlNotes)
                {
                    theMidiLoop.Key[SelectedKeyNote].Part[int(Time * TimeMult) % Quantize].Midi.insertLast(bl.inputMidiEvents[i]);
                }

                if (LevelSelected == kLvlPhrase)
                {
                    if (_ty == kMidiNoteOn) thePhraseLoop.Key[SelectedKeyPhrase].Part[int(Time / 4) % 4].Keys.insertLast( int(MidiEventUtils::getNote(bl.inputMidiEvents[i])) );
                }

                if (LevelSelected == kLvlSection)
                {
                    if (_ty == kMidiNoteOn) theSectionLoop.Key[SelectedKeySection].Part[int(Time / 16) % 4].Keys.insertLast( int(MidiEventUtils::getNote(bl.inputMidiEvents[i])) );
                }

                if (LevelSelected == kLvlSong)
                {
                    if (_ty == kMidiNoteOn) theSongLoop.Key[SelectedKeySong].Keys.insertLast( int(MidiEventUtils::getNote(bl.inputMidiEvents[i])) );
                }

                if (LevelSelected == kLvlAlbum)
                {
                    if (_ty == kMidiNoteOn) theAlbumLoop.Key[SelectedKeyAlbum].Keys.insertLast( int(MidiEventUtils::getNote(bl.inputMidiEvents[i])) );
                }
            }

            // Overview Mode (not edit mode)                
            else if (_ty == kMidiNoteOn)
            {
                if (LevelSelected == kLvlNotes) SelectedKeyNote = MidiEventUtils::getNote(bl.inputMidiEvents[i]);
                if (LevelSelected == kLvlPhrase) SelectedKeyPhrase = MidiEventUtils::getNote(bl.inputMidiEvents[i]);
                if (LevelSelected == kLvlSection) SelectedKeySection = MidiEventUtils::getNote(bl.inputMidiEvents[i]);
                if (LevelSelected == kLvlSong) SelectedKeySong = MidiEventUtils::getNote(bl.inputMidiEvents[i]);
                if (LevelSelected == kLvlAlbum) SelectedKeyAlbum = MidiEventUtils::getNote(bl.inputMidiEvents[i]);

                // all notes on are shut off between transitions
                AllNotesOff();  
            }
        }


        // ========================================
        // Output - Play the Notes
        // ========================================
        // -- no need to do this for every sample.. this can be removed, just iterate through the quantization steps only, it will speed up dramatically
        // -- simply use the offset of the midi message to determine where it belongs
        // or in this case, find the range it covers and loop only across those ones
        for (int i = 0; i < int(bl.samplesToProcess); ++i) 
        {
            Time = samplesToQuarterNotes(InternalSampleCount,bl.transport.bpm);

            if (lastQuater != int(Time * TimeMult))
            {
                lastQuater = int(Time * TimeMult);

                if (LevelSelected == kLvlNotes)
                {                    
                    PlayNotes(SelectedKeyNote);

                    // Metronome
                    //if (EditMode) {}
                    // perhaps a starting beep metronode is played so people know when the loop starts and the pace
                    // the metronome does not appear in the output loop, and does note play if a note on exists in the same space
                    //-- metronome plays until a note on replaces it, the metronome is not recorded, only acts as a placeholder
                    //-- first note oct different then others
                    if (Time % 4 == 0 && EditMode)
                    SendMetronome((theMidiLoop.Key[SelectedKeyNote].Part[lastQuater % Quantize].Midi.length != 0));
                }

                // Play from the currently selected key
                if (LevelSelected == kLvlPhrase) PlayPhrase(SelectedKeyPhrase); 
                if (LevelSelected == kLvlSection) PlaySection(SelectedKeySection);
                if (LevelSelected == kLvlSong) PlaySong(SelectedKeySong);
                if (LevelSelected == kLvlAlbum) PlayAlbum(SelectedKeyAlbum);
            }

            InternalSampleCount++;
        }    
}

void PlayNotes(int KeyToPlay)
{
    if (theMidiLoop.Key[KeyToPlay].Part[lastQuater % Quantize].Midi.length != 0)
        for(uint k=0;k<theMidiLoop.Key[KeyToPlay].Part[lastQuater % Quantize].Midi.length;k++)
            bl.outputMidiEvents.push(theMidiLoop.Key[KeyToPlay].Part[lastQuater % Quantize].Midi[k]);

    // have one for 1 bar, one for 2 bars, one for 4 bars, have a loop for each type (1,2,4)
}

void PlayPhrase(int KeyToPlay)
{
    int CurrentTime = int(Time * 0.25) % 4;
    for (uint k = 0; k < thePhraseLoop.Key[KeyToPlay].Part[CurrentTime].Keys.length; ++k)
        PlayNotes(thePhraseLoop.Key[KeyToPlay].Part[CurrentTime].Keys[k]);
}

void PlaySection(int KeyToPlay)
{
    int CurrentTime = int(Time * 0.0625) % 4;
    for (uint k = 0; k < theSectionLoop.Key[KeyToPlay].Part[CurrentTime].Keys.length; ++k)
        PlayPhrase(theSectionLoop.Key[KeyToPlay].Part[CurrentTime].Keys[k]);
}

void PlaySong(int KeyToPlay)
{
    if (GetSongLength(KeyToPlay) > 0) PlaySection(theSongLoop.Key[KeyToPlay].Keys[int(Time * 0.015625) % GetSongLength(KeyToPlay)]);
}

void PlaySongOffset(int KeyToPlay, int Offset)
{
     if (GetSongLength(KeyToPlay) > 0) PlaySection(theSongLoop.Key[KeyToPlay].Keys[(int(Time * 0.015625)+Offset) % GetSongLength(KeyToPlay)]);
}

void PlayAlbum(int KeyToPlay)
{
    if (theAlbumLoop.Key[KeyToPlay].Keys.length > 0)
    {
        int SongAccum = 0;
        int ActiveSong = 0;
        int ActiveSongOff = 0;
        int CurrentTime = int(Time * 0.015625) % GetAlbumLength(KeyToPlay);

        for (uint i = 0; i < theAlbumLoop.Key[KeyToPlay].Keys.length; ++i)
        {
            if (CurrentTime >= SongAccum)
            {
                ActiveSong = i;
                ActiveSongOff = SongAccum;
            }
            SongAccum += GetSongLength(theAlbumLoop.Key[KeyToPlay].Keys[i]);        
        }

        print("ActiveSong: " + ActiveSong + ", ActiveSongOff: " + ActiveSongOff + ", KeyToPlay: " + KeyToPlay);
        PlaySongOffset(theAlbumLoop.Key[KeyToPlay].Keys[ActiveSong], ActiveSongOff);
    }
}

int GetSongLength(int KeyLength)
{
    return theSongLoop.Key[KeyLength].Keys.length;
}

int GetAlbumLength(int KeyLength)
{
    int AlbumLengthSum = 0;
    for (uint i = 0; i < theAlbumLoop.Key[KeyLength].Keys.length; ++i)
        AlbumLengthSum += GetSongLength(theAlbumLoop.Key[KeyLength].Keys[i]);
    return AlbumLengthSum;
}

void AllNotesOff()
{
    MidiEvent noteOut;
    noteOut.byte0 = 0x80 | (noteOut.byte0 & 0x0F);    
    noteOut.byte2 = 127 & 0x7F;    
    for (int i = 0; i < 127; ++i)
    {
        noteOut.byte1 = (i & 0x7F);
        bl.outputMidiEvents.push(noteOut);
    }
}

double samplesToQuarterNotes(double samples,double bpm)
{
    return samples*bpm/(60.0*sampleRate);
}

bool Enabled = true;
bool FreeRun = false;

bool SentOnLast = false;
void SendMetronome(bool Well)
{
    MidiEvent noteOut;
    noteOut.byte2 = 127 & 0x7F;    
    noteOut.byte1 = 60 & 0x7F;

    if (SentOnLast)
    {
        noteOut.byte0 = 0x80 | (noteOut.byte0 & 0x0F);
        bl.outputMidiEvents.push(noteOut);
    }

    if (!Well)
    {
        noteOut.byte0 = 0x90 | (noteOut.byte0 & 0x0F);
        bl.outputMidiEvents.push(noteOut);
        SentOnLast = true;
    }
}

void updateInputParameters()
{
    // Make these states tied to keyboard commands, not daw
    Enabled = true;
    if (inputParameters[0] < 0.01) Enabled = false;
    FreeRun = false;
    if (inputParameters[0] > 0.5) FreeRun = true;
}


/*
Notes
=========
Simple as possible...

Metronomes are vital to playing in loops , so it should defintiely send a pacing signal..
Keyboard only function.

    Level 1 Record raw notes in loops
    Level 2 Composition Level 1 together into larger loops
    Level 3 Seqeunce Level 2 one after the next

    Key Combination Commands -  
    Level Change C + D E F (send midi note/cc to indicate level by number of beeps)
    Current Level C + D + E + F (send midi note/cc to indicate level by number of beeps)
    Ping C + D + E + F + G + A + B, long duration - -system cuts all notes and sends init sequence (to hear it)

    Will Record until it loops without a keypress entirely once then exits edit mode

    holding down the key without anything will preview the key, 
    long and short key presses are noticed, and when entering and exiting recording mode, it will send another midi note with it
    metronome plays until replaced by midi notes
    if long inactivity occurs, the click accumulator turns back to zero

    Level 1
        -- Wihtout clicking, holding down a note is a command
        -- Recording Length in Bars on number of repetitions of keypresses, then hold it down to start recording/Clear
        -- Convert notes to chords.. quantitzation level.. hold down note and octave above it, reptition click number is size of quantitzation
        -- automatically cut notes at end of clip
        -- when holding down, a beep should occur, or a simple midi output..
    Level 2
        -- key must be held down for continuous recording of level 1 material, else a restart occurs
        -- Recording Length in Bars on number of repetitions of keypresses, then hold it down to start recording/Clear

    Level 3
        -- Sequences together level 2 Material (one per key)

    Keep Quanitization level at 16ths for everything.. for simplicity

    In Recording Mode it iwll play a minimum of twice. 
    and if no notes are recieved during a loop it will end (other than the first one)
    notes are immeidately added to the loop
    // notes are turned off at the end of the loop duration

*/

