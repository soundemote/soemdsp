#pragma once

#include "Wire.hpp"
#include "sehelper.hpp"
#include "semath.hpp"

#include <fmt/format.h>
// #include <gsl/pointers>
// #include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <map>
#include <source_location>
#include <vector>

namespace soemdsp {

enum class MidiMessage {
    note_off,
    note_on,
    allnotesoff,
    controller, // continuous controller
    pitchbend,
    aftertouch,
    pressure, // channel pressure
    program   // program change
};

template<typename voice_t>
struct MidiState {
    voice_t* voice_{}; // current voice

    MidiMessage midiMessage_{};

    int note_{ -1 }; // 0 to 127

    double velocity_{ -1.0 }; // 0.0 to 1.0

    double aftertouch_{ -1.0 }; // 0.0 to 1.0

    double channelPressure_{ -1.0 }; //  0.0 to 1.0

    int ccIndex_{ -1 }; // 0 to 127
    double ccValue_{};  // 0.0 to 1.0

    int sampleTimeStamp_{}; // timestamp in samples of the midi event for per-block scheduling into per-sample
};

typedef struct SoEmVstEvent {
    //-------------------------------------------------------------------------------------------------------
    int type{};        ///< @see VstEventTypes
    int byteSize{};    ///< size of this event, excl. type and byteSize
    int deltaFrames{}; ///< sample frames related to the current block start sample position
    int flags{};       ///< generic flags, none defined yet

    char data[16]; ///< data size may vary, depending on event type
    //-------------------------------------------------------------------------------------------------------
} SoEmVstEvent;
struct SoEmVstEvents {
    //-------------------------------------------------------------------------------------------------------

    int numEvents{};                     ///< number of Events in array
    __int64* reserved{};                 ///< zero (Reserved for future use)
    SoEmVstEvent events[2] = { {}, {} }; ///< event pointer array, variable size
    //-------------------------------------------------------------------------------------------------------
};
struct SoEmVstMidiEvent {
    //-------------------------------------------------------------------------------------------------------
    int type{};             ///< #kVstMidiType
    int byteSize{};         ///< sizeof (VstMidiEvent)
    int deltaFrames{};      ///< sample frames related to the current block start sample position
    int flags{};            ///< @see VstMidiEventFlags
    int noteLength{};       ///< (in sample frames) of entire note, if available, else 0
    int noteOffset{};       ///< offset (in sample frames) into note from note start if available, else 0
    char midiData[4];       ///< 1 to 3 MIDI bytes; midiData[3] is reserved (zero)
    char detune{};          ///< -64 to +63 cents; for scales other than 'well-tempered' ('microtuning')
    char noteOffVelocity{}; ///< Note Off Velocity [0, 127]
    char reserved1{};       ///< zero (Reserved for future use)
    char reserved2{};       ///< zero (Reserved for future use)
    //-------------------------------------------------------------------------------------------------------
};

// specify the voice class for voice_t and number of audio channels for a voice and maximum polyphony as a size
template<class voice_t, class enum_t, size_t audioChannelSize, size_t maxVoicesSize>
class VoiceManager {
    // using VoicePtr = gsl::not_null<voice_t*>;
    using VoicePtr = voice_t*;
    using VoiceRef = voice_t&;

  public:
    enum class PhonyMode {
        monophony, // allow only one voice at a time
        polyphony, // allow multiple voices at a time
    };

    enum class SlideMode {
        never_slide, // don't slide notes in monophony mode
        allow_slide, // only slide notes when a note is already held down
        always_slide // always slide notes even when a note is not held down
    };

    enum class SlideCurve {
        linear,
        expo,
        log,
        sigmoid
    };

    enum class ReleasingStealingMode {
        oldest, // steal oldest releasing voice
        newest, // steal newest releasing voice
    };

    enum class SustainingStealingMode {
        oldest,  // steal oldest sustaining voice
        newest,  // steal newest sustaining voice
        closest, // steal closest-to-previously-played sustaining voice biased to higher notes
        lowest,  // steal lowest-played sustaining voice
        highest  // steal highest-played sustaining voice
    };

    enum class ControlMode {
        Keyboard,
        AlwaysOn
    };

    void setPitchBendRange(double range) {
        if (pitchBendRange_ == range) {
            return;
        }

        pitchBendRange_ = range;
        updatePitchBendValue();
    }

    double getPitchBendValue() {
        return pitchBendValue_;
    }

    // IMPORTANT: This callback must be defined or the release stage will instantly cut the voice. This for checking if
    // a releasing voice still needs processing, for example envelopes that have triggered a release.
    std::function<bool(voice_t*)> isIdleCallback_ = [](voice_t*) { return true; };
    // IMPORTANT: This callback must be defined in order to get any audio.
    std::function<std::array<double, audioChannelSize>(voice_t*)> runCallback_ = [](voice_t*) { return std::array<double, audioChannelSize>{ 0 }; };

    // called for every midi note on event, DO NOT use this callback for starting a voice, see attackCallback
    std::function<void(MidiState<voice_t>*)> noteOnCallback_ = [](MidiState<voice_t>*) {};
    // called for every midi note on event, DO NOT use this callback for ending a voice, see releaseCallback
    std::function<void(MidiState<voice_t>*)> noteOffCallback_ = [](MidiState<voice_t>*) {};

    // triggered for a note on when a new attack is happening, e.g. there are no notes held, but also depends on other
    // modes
    std::function<void(MidiState<voice_t>*)> attackCallback_ = [](MidiState<voice_t>*) {};
    // triggered for legato, e.g. when notes are held and a new note is played for sliding to new pitch, does not add
    // voices
    std::function<void(MidiState<voice_t>*)> legatoCallback_ = [](MidiState<voice_t>*) {};
    // triggered for "always_slide" mode, start the pitch at the initial or previous pitch and slide to desired pitch
    std::function<void(MidiState<voice_t>*)> slideCallback_ = [](MidiState<voice_t>*) {};
    // triggered on non-legato note-offs, removes a voice from the releasing vector
    std::function<void(MidiState<voice_t>*)> releaseCallback_ = [](MidiState<voice_t>*) {};
    // triggered for 'all notes off' midi messages, removes all voices from sustaining and releasing
    std::function<void(MidiState<voice_t>*)> allNotesOffCallback_ = [](MidiState<voice_t>*) {};
    // triggered for a pitch bend midi signal
    std::function<void(MidiState<voice_t>*)> pitchbendCallback_ = [](MidiState<voice_t>*) {};
    // triggered for a continuous controller midi signal
    std::function<void(MidiState<voice_t>*)> controllerCallback_ = [](MidiState<voice_t>*) {};
    // triggered for monophonic (every notes has same value) channelpressure
    std::function<void(MidiState<voice_t>*)> channelpressureCallback_ = [](MidiState<voice_t>*) {};
    // triggered for polyphonic (every note has individual value) aftertouch
    std::function<void(MidiState<voice_t>*)> aftertouchCallback_ = [](MidiState<voice_t>*) {};

    void addMidiEvent(MidiState<voice_t> state) {
        eventSchedule_.push_back(state);
    }

    int noteForAlwaysOn_ = 48;
    void setControlMode(ControlMode mode) {
        endAllVoices();
        eventSchedule_.clear();
        controlMode_ = mode;
        if (controlMode_ == ControlMode::AlwaysOn) {
            MidiState<voice_t> state;
            state.midiMessage_     = MidiMessage::note_on;
            state.note_            = noteForAlwaysOn_;
            state.velocity_        = 1.0;
            state.sampleTimeStamp_ = samplesSinceBlock_;
            addMidiEvent(state);
        }
    }

    int kVstMidiType{ 1 };
    void ProcessVstMidiEvents(SoEmVstEvents* ev) {
        for (int i = 0; i < ev->numEvents; ++i) {
            if (ev->events[i].type != kVstMidiType) {
                continue;
            }

            auto* midiData = std::bit_cast<SoEmVstMidiEvent>(ev->events[i]).midiData;

            // ignore note offs if ControlMode is always on
            if (controlMode_ == ControlMode::AlwaysOn && (midiData[0] & 0xF0) == 0x80) {
                continue;
            } else if (controlMode_ == ControlMode::AlwaysOn && (midiData[0] & 0xF0) == 0x90) {
                endAllVoices();
            }

            // Make a new state object and assign state object to "current state" pointer
            eventSchedule_.push_back({});
            midiState_ = &eventSchedule_.back();

            // Assign time for the midi event
            midiState_->sampleTimeStamp_ = ev->events[i].deltaFrames;

            switch (midiData[0] & 0xF0) { // NOLINT
            case 0x80:                    // NOLINT
                midiState_->midiMessage_ = MidiMessage::note_off;
                midiState_->note_        = static_cast<int>(midiData[1] & 0x7F);
                midiState_->velocity_    = (midiData[2] & 0x7F) * k1z127;
                break;
            case 0x90: // NOLINT
                midiState_->midiMessage_ = MidiMessage::note_on;
                midiState_->note_        = static_cast<int>(midiData[1] & 0x7F);
                midiState_->velocity_    = (midiData[2] & 0x7F) * k1z127;
                SE_ERROR(polyphony_ > 0, "You must set polyphony to be more than 0 in order for VoiceManager to function.");
                break;
            case 0xB0:
                midiState_->midiMessage_ = MidiMessage::controller;
                midiState_->ccIndex_     = static_cast<int>(midiData[1] & 0x7F);
                midiState_->ccValue_     = midiData[2] * k1z127;
                break;
            case 0xC0: // NOLINT
                midiState_->midiMessage_ = MidiMessage::program;
                break;
            case 0xE0: { // NOLINT
                midiState_->midiMessage_   = MidiMessage::pitchbend;
                int pitchBendMidiValue     = midiData[1] | (midiData[2] << 7);
                double pitchBendNormalized = toDouble(pitchBendMidiValue) * k1z16384;
                pitchBendPosition_         = uniToBi(pitchBendNormalized);
            } break;
            case 0xA0: // NOLINT
                midiState_->midiMessage_ = MidiMessage::aftertouch;
                midiState_->aftertouch_  = 0; // TODO():input right numbers
                break;
            case 0xD0: // NOLINT
                midiState_->midiMessage_     = MidiMessage::pressure;
                midiState_->note_            = static_cast<int>(midiData[1] & 0x7F);
                midiState_->channelPressure_ = midiData[2] * k1z127;
                break;
            }
        }
    }

    // bankOfVoices is a vector of voice objects that the VoiceManager can draw from for polyphony
    void setPolyphony(int polyphony) {
        SE_WITHIN_SIZE(polyphony, (*voiceBank_))

        if (polyphony_ == polyphony) {
            return;
        }

        eventSchedule_.clear();
        availableVoices_.clear();
        sustainingVoices_.clear();
        releasingVoices_.clear();
        midiNoteStatus_.fill(false);
        noteHistory_.clear();
        indexForLegato_ = 0;

        for (int i = 0; i < polyphony; ++i) {
            availableVoices_.push_back(&(*voiceBank_)[i]);
        }

        // update new voices' parameters
        for (size_t i = polyphony_; i < polyphony; ++i) {
            (*voiceBank_)[i].dirtyUpdater_.updateAllIfNeeded();
        }

        polyphony_ = polyphony;
    }

    // remove voices that no longer need processing, recommended to be called every block
    void clean() {
        // find all voices that match isIdle
        auto iter = std::partition(std::begin(releasingVoices_), std::end(releasingVoices_), [&](voice_t* voice) { return !isIdleCallback_(voice); });
        // add found voices to available
        std::copy(iter, std::end(releasingVoices_), std::back_inserter(availableVoices_));
        // remove found voices from releasing
        releasingVoices_.erase(iter, std::end(releasingVoices_));

        // finished processing all events
        if (sustainingVoices_.empty() && releasingVoices_.empty()) {
            currentEventIndex_ = 0;
            eventSchedule_.clear();
        }

        samplesSinceBlock_ = 0;
    }

    MidiState<voice_t>* midiState_; // current state
    PhonyMode phonyMode_{ PhonyMode::monophony };
    SlideMode slideMode_{ SlideMode::never_slide };
    ControlMode controlMode_{ ControlMode::Keyboard };
    ReleasingStealingMode releasingStealingMode_{ ReleasingStealingMode::oldest };
    SustainingStealingMode sustainingStealingMode_{ SustainingStealingMode::oldest };

    std::array<voice_t, maxVoicesSize>* voiceBank_{};
    std::vector<VoicePtr> availableVoices_;
    std::vector<VoicePtr> sustainingVoices_;
    std::vector<VoicePtr> releasingVoices_;
    std::vector<MidiState<voice_t>> eventSchedule_;
    std::array<bool, 128> midiNoteStatus_{ false };
    std::array<double, 128> midiControllerValue_{ 0.0 };

    std::array<double, audioChannelSize> run() {
        if (currentEventIndex_ < eventSchedule_.size() && eventSchedule_[currentEventIndex_].sampleTimeStamp_ == samplesSinceBlock_) {
            while (currentEventIndex_ < eventSchedule_.size() && eventSchedule_[currentEventIndex_].sampleTimeStamp_ == samplesSinceBlock_) {
                midiState_ = &eventSchedule_[currentEventIndex_];
                switch (midiState_->midiMessage_) {
                default:
                    SE_FAIL(fmt::format("Received unhandled event:{}", static_cast<int>(midiState_->midiMessage_)));
                case MidiMessage::note_on: // only process note on if note is not already on
                    if (midiNoteStatus_[midiState_->note_] != true) {
                        midiNoteStatus_[midiState_->note_] = true;
                        addNoteToHistory(midiState_->note_);
                        switch (phonyMode_) {
                        default:
                            SE_FAIL(fmt::format("Received unhandled event:{}", static_cast<int>(phonyMode_)));
                        case PhonyMode::monophony:
                            handleMonophonyNoteOn();
                            break;
                        case PhonyMode::polyphony:
                            handlePolyphonyNoteOn();
                            break;
                        }
                        previousNoteOn_ = midiState_->note_;
                    }
                    break;
                case MidiMessage::note_off: // only process note off if note is not already off
                    if (midiNoteStatus_[midiState_->note_] != false) {
                        midiNoteStatus_[midiState_->note_] = false;
                        removeNoteFromHistory(midiState_->note_);
                        switch (phonyMode_) {
                        default:
                            SE_FAIL(fmt::format("Received unhandled event:{}", static_cast<int>(phonyMode_)));
                        case PhonyMode::monophony:
                            handleMonophonyNoteOff();
                            break;
                        case PhonyMode::polyphony:
                            handlePolyphonyNoteOff();
                            break;
                        }
                    }
                    break;
                case MidiMessage::controller:
                    midiControllerValue_[midiState_->ccIndex_] = midiState_->ccValue_;
                    controllerCallback_(midiState_);
                    break;
                case MidiMessage::allnotesoff:
                    allNotesOffCallback_(midiState_);
                    break;
                case MidiMessage::pitchbend:
                    updatePitchBendValue();
                    pitchbendCallback_(midiState_);
                    break;
                case MidiMessage::aftertouch:
                    aftertouchCallback_(midiState_);
                    break;
                case MidiMessage::pressure:
                    channelpressureCallback_(midiState_);
                    break;
                }
                ++currentEventIndex_;
            }
        } else {
            ++samplesSinceBlock_;
        }

        std::array<double, audioChannelSize> out{ 0.0 };

        for (auto& voice : sustainingVoices_) {
            std::array<double, audioChannelSize> arr = runCallback_(voice);
            // accumulate sustainingVoices_ values into out values
            std::transform(std::cbegin(arr), std::cend(arr), std::cbegin(out), std::begin(out), std::plus<double>{});
        }
        for (auto& voice : releasingVoices_) {
            std::array<double, audioChannelSize> arr = runCallback_(voice);
            // accumulate releasingVoices_ values into out values
            std::transform(std::cbegin(arr), std::cend(arr), std::cbegin(out), std::begin(out), std::plus<double>{});
        }
        return out;
    }

    void markAllVoicesDirty(enum_t flag) {
        for (auto& voice : availableVoices_) {
            voice->dirtyUpdater_.markDirty(flag);
        }
        for (auto& voice : sustainingVoices_) {
            voice->dirtyUpdater_.markDirty(flag);
        }
        for (auto& voice : releasingVoices_) {
            voice->dirtyUpdater_.markDirty(flag);
        }
    }
    void updateAllVoices(enum_t flag) {
        for (auto& voice : availableVoices_) {
            voice->dirtyUpdater_.markDirty(flag);
        }
        for (auto& voice : sustainingVoices_) {
            voice->dirtyUpdater_.update(flag);
        }
        for (auto& voice : releasingVoices_) {
            voice->dirtyUpdater_.update(flag);
        }
    }
    void updateVoiceAllFlagsIfNeeded(VoicePtr voice) {
        voice->dirtyUpdater_.updateAllIfNeeded();
    }

    double pitchBendValue_{};

  private:
    int samplesSinceBlock_{ 0 };
    int currentEventIndex_{ 0 };
    int polyphony_{ -1 };
    double pitchBendRange_{};
    double pitchBendPosition_{}; // -1.0 to +1.0
    // 0 index is previous note on, 1 index is the 2nd to previous of note on
    std::vector<int> noteHistory_;
    int previousNoteOn_{};
    size_t indexForLegato_{};

    void addNoteToHistory(int note) {
        noteHistory_.insert(noteHistory_.begin(), note);
    }
    void removeNoteFromHistory(int note) {
        auto iter = find_if(noteHistory_.begin(), noteHistory_.end(), [note](int noteHistoryNote) { return noteHistoryNote == note; });

        if (iter != noteHistory_.end()) {
            noteHistory_.erase(iter);
        }
    }

    void updatePitchBendValue() {
        double newVal = pitchBendPosition_ * pitchBendRange_;

        if (pitchBendValue_ == newVal) {
            return;
        }

        pitchBendValue_ = newVal;
        MidiState<voice_t> state;
        state.midiMessage_ = MidiMessage::pitchbend;
        addMidiEvent(state);
    }

    // trigger based on if a note is held and another note is added or when a note is held and another note is released
    void handleMonophonyNoteOn() {
        switch (slideMode_) {
        default:
            SE_FAIL(fmt::format("Received unhandled event:{}", static_cast<int>(slideMode_)));
        case SlideMode::never_slide:
            // note on message
            midiNoteStatus_[midiState_->note_] = true;
            midiState_->voice_                 = moveVoiceTo(sustainingVoices_, indexForLegato_);
            midiState_->voice_->note_          = midiState_->note_;
            midiState_->voice_->velocity_      = midiState_->velocity_;
            updateVoiceAllFlagsIfNeeded(midiState_->voice_);
            noteOnCallback_(midiState_);
            attackCallback_(midiState_);
            break;
        case SlideMode::allow_slide:
            if (sustainingVoices_.empty()) { // No voices active
                midiState_->voice_            = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->voice_->note_     = midiState_->note_;
                midiState_->voice_->velocity_ = midiState_->velocity_;
                updateVoiceAllFlagsIfNeeded(midiState_->voice_);
                noteOnCallback_(midiState_);
                attackCallback_(midiState_);
            } else { // A voice is already active
                midiState_->voice_            = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->voice_->note_     = midiState_->note_;
                midiState_->voice_->velocity_ = midiState_->velocity_;
                updateVoiceAllFlagsIfNeeded(midiState_->voice_);
                noteOnCallback_(midiState_);
                legatoCallback_(midiState_);
            }
            break;
        case SlideMode::always_slide:
            if (sustainingVoices_.empty()) { // No voices active
                midiState_->voice_            = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->voice_->note_     = midiState_->note_;
                midiState_->voice_->velocity_ = midiState_->velocity_;
                updateVoiceAllFlagsIfNeeded(midiState_->voice_);
                noteOnCallback_(midiState_);
                slideCallback_(midiState_);
            } else { // Voice is already active
                midiState_->voice_            = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->voice_->note_     = midiState_->note_;
                midiState_->voice_->velocity_ = midiState_->velocity_;
                updateVoiceAllFlagsIfNeeded(midiState_->voice_);
                noteOnCallback_(midiState_);
                legatoCallback_(midiState_);
            }
            break;
        }
    }

    // trigger based on if a note is held and another note is added or when a note is held and another note is released
    void handleMonophonyNoteOff() {
        switch (slideMode_) {
        default:
            SE_FAIL(fmt::format("Received unhandled event:{}", static_cast<int>(slideMode_)));
        case SlideMode::never_slide: {
            if (noteHistory_.empty()) { // No notes are being held
                midiState_->voice_        = moveVoiceTo(releasingVoices_, indexForLegato_);
                midiState_->voice_->note_ = midiState_->note_;
                noteOffCallback_(midiState_);
                releaseCallback_(midiState_);
            } // note is activated and released without other events inbetween
            else if (previousNoteOn_ == midiState_->note_) {
                midiState_->voice_        = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->note_         = noteHistory_[0];
                midiState_->voice_->note_ = midiState_->note_;
                noteOnCallback_(midiState_);
                attackCallback_(midiState_);
            }
        } break;
        case SlideMode::allow_slide: {
            if (noteHistory_.empty()) { // No notes are being held
                midiState_->voice_        = moveVoiceTo(releasingVoices_, indexForLegato_);
                midiState_->voice_->note_ = midiState_->note_;
                noteOffCallback_(midiState_);
                releaseCallback_(midiState_);
            } else if (previousNoteOn_ != midiState_->note_) { // note is activated and released without other events //
                                                               // inbetween
                midiState_->voice_        = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->note_         = noteHistory_[0];
                midiState_->voice_->note_ = midiState_->note_;
                noteOffCallback_(midiState_);
            } else { // off note legato
                midiState_->voice_        = moveVoiceTo(sustainingVoices_, indexForLegato_);
                midiState_->note_         = noteHistory_[0];
                midiState_->voice_->note_ = midiState_->note_;
                noteOffCallback_(midiState_);
                previousNoteOn_ = midiState_->note_;
                legatoCallback_(midiState_);
            }
        } break;
        case SlideMode::always_slide: {
            if (noteHistory_.empty()) { // No notes are being held or latest note is being unheld
                midiState_->voice_        = moveVoiceTo(releasingVoices_, indexForLegato_);
                midiState_->voice_->note_ = midiState_->note_;
                noteOffCallback_(midiState_);
                releaseCallback_(midiState_);
            }
        } break;
        }
    }

    void handlePolyphonyNoteOn() {
        switch (slideMode_) {
        default:
            SE_FAIL(fmt::format("Received unhandled event:{}", static_cast<int>(slideMode_)));
        case SlideMode::never_slide:
        case SlideMode::allow_slide: {
            midiState_->voice_            = addVoice();
            midiState_->voice_->note_     = midiState_->note_;
            midiState_->voice_->velocity_ = midiState_->velocity_;
            updateVoiceAllFlagsIfNeeded(midiState_->voice_);
            noteOnCallback_(midiState_);
            attackCallback_(midiState_);
        } break;
        case SlideMode::always_slide: {
            midiState_->voice_            = addVoice();
            midiState_->voice_->note_     = midiState_->note_;
            midiState_->voice_->velocity_ = midiState_->velocity_;
            updateVoiceAllFlagsIfNeeded(midiState_->voice_);
            noteOnCallback_(midiState_);
            slideCallback_(midiState_);
        } break;
        }
        indexForLegato_ = midiState_->voice_->idx_;
    }
    void handlePolyphonyNoteOff() {
        midiState_->voice_ = releaseVoiceByNote(midiState_->note_);
        if (midiState_->voice_ != nullptr) {
            noteOffCallback_(midiState_);
            releaseCallback_(midiState_);
        }
    }

    voice_t* addVoice() {
        voice_t* voice{};

        if (availableVoices_.empty()) {
            if (releasingVoices_.empty()) { // Steal from sustaining
                switch (sustainingStealingMode_) {
                case SustainingStealingMode::oldest:
                    voice = sustainingVoices_.front();
                    sustainingVoices_.erase(sustainingVoices_.begin());
                    sustainingVoices_.push_back(voice);
                    break;
                case SustainingStealingMode::newest:
                    voice = sustainingVoices_.back();
                    break;
                case SustainingStealingMode::closest:
                    // TODO(): implement
                    break;
                case SustainingStealingMode::lowest:
                    // TODO(): implement
                    break;
                case SustainingStealingMode::highest:
                    // TODO(): implement
                    break;
                }
            } else { // Steal from releasing
                if (reuseSameNoteVoice_ == 1) {
                    voice = stealReleasingVoiceFromSustaining(midiState_->note_);
                }
                if (voice == nullptr) {
                    switch (releasingStealingMode_) {
                    case ReleasingStealingMode::oldest:
                        voice = releasingVoices_.front();
                        releasingVoices_.erase(releasingVoices_.begin());
                        break;
                    case ReleasingStealingMode::newest:
                        voice = releasingVoices_.back();
                        releasingVoices_.erase(releasingVoices_.end());
                        break;
                    }
                    sustainingVoices_.push_back(voice);
                }
            }
        } else { // Get from available
            if (reuseSameNoteVoice_ == 1) {
                voice = stealReleasingVoiceFromSustaining(midiState_->note_);
            }
            if (voice == nullptr) {
                voice = availableVoices_.front();
                sustainingVoices_.push_back(availableVoices_.front());
                availableVoices_.erase(availableVoices_.begin());
            }
        }

        return voice;
    }

    voice_t* releaseVoiceByNote(int note) {
        voice_t* voice{};

        auto iter = find_if(sustainingVoices_.begin(), sustainingVoices_.end(), [&](VoicePtr v) { return v->note_ == note; });

        if (iter != sustainingVoices_.end()) {
            voice = *iter;
            releasingVoices_.push_back(*iter);
            sustainingVoices_.erase(iter);
        }

        return voice;
    }

    void moveVoiceTo(std::vector<VoicePtr>& containerA, std::vector<VoicePtr>& containerB, VoicePtr voiceToFind) {
        if (containerA == containerB) {
            return;
        }

        auto iter = std::find(containerA.begin(), containerA.end(), voiceToFind);

        SE_ERROR(iter != containerA.end(), "voice object not found in containerA");

        containerA.erase(iter);
        containerB.push_back(voiceToFind);
    }

    VoicePtr moveVoiceTo(std::vector<VoicePtr>& container, size_t voiceIdxToUse) {
        std::vector<VoicePtr>* containerRef{ nullptr };
        auto voice = findVoiceByIndex(voiceIdxToUse, containerRef);
        moveVoiceTo(*containerRef, container, voice);
        return voice;
    }

    // returns the voice based on the original "availableVoices_" index and optionally returns the container it was
    // found in
    VoicePtr findVoiceByIndex(size_t idx, std::vector<VoicePtr>*& containerOut = nullptr) {
        auto iter1 = find_if(availableVoices_.begin(), availableVoices_.end(), [&](VoicePtr v) { return v->idx_ == idx; });
        if (iter1 != availableVoices_.end()) {
            containerOut = &availableVoices_;
            return *iter1;
        }

        auto iter2 = find_if(sustainingVoices_.begin(), sustainingVoices_.end(), [&](VoicePtr v) { return v->idx_ == idx; });
        if (iter2 != sustainingVoices_.end()) {
            containerOut = &sustainingVoices_;
            return *iter2;
        }

        auto iter3 = find_if(releasingVoices_.begin(), releasingVoices_.end(), [&](VoicePtr v) { return v->idx_ == idx; });
        if (iter3 != releasingVoices_.end()) {
            containerOut = &releasingVoices_;
            return *iter3;
        }

        SE_FAIL("index does not exist in any voice container! why would you request an index that doesn't exist?");
        return *iter1;
    }

    voice_t* stealReleasingVoiceFromSustaining(int note) {
        auto iter = find_if(releasingVoices_.begin(), releasingVoices_.end(), [&](VoicePtr v) { return v->note_ == note; });

        if (iter != releasingVoices_.end()) {
            sustainingVoices_.push_back(*iter);
            releasingVoices_.erase(iter);
            return sustainingVoices_.back();
        }

        return nullptr;
    }

    void endAllVoices() {
        std::copy(std::begin(sustainingVoices_), std::end(sustainingVoices_), std::back_inserter(availableVoices_));
        std::copy(std::begin(releasingVoices_), std::end(releasingVoices_), std::back_inserter(availableVoices_));

        sustainingVoices_.clear();
        releasingVoices_.clear();
        midiNoteStatus_.fill(false);
        noteHistory_.clear();
    }

    /* Options */
    Wire<int> reuseSameNoteVoice_;
};
} // namespace soemdsp
