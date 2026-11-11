// ES9 CV — framework-independent CV/gate engine.
//
// Converts MIDI-style events and host transport into DC "audio" samples for
// the Expert Sleepers ES-9, whose DC-coupled outputs produce +10 V for a
// digital sample value of 1.0. This file has no JUCE dependency so it can be
// unit-tested on any platform.
#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace es9cv {

constexpr double kFullScaleVolts = 10.0;   // ES-9: sample 1.0 == +10 V
constexpr int kNumChannels = 2;            // one instance = one stereo pair

enum class Function : int {
    Off = 0,
    Pitch,        // 1 V/oct from the mono voice (+ bend, glide)
    Gate,         // high while a note is held
    Trigger,      // short pulse on note-on
    Velocity,     // last note-on velocity scaled to min..max V
    CC,           // a MIDI CC scaled to min..max V (smoothed)
    Clock,        // tempo-synced pulses while the transport runs
    Run,          // high while the transport runs
    Reset,        // pulse on transport start and on loop/jump back
    LFO,          // tempo-synced LFO between min..max V
    TestVoltage,  // constant voltage: Phase 0 DC test and calibration
    Count
};

enum class LfoShape : int { Sine = 0, Triangle, SawUp, SawDown, Square, Count };

struct ChannelSettings {
    Function function = Function::Off;
    int ccNumber = 1;
    double minVolts = 0.0;        // range for Velocity / CC / LFO
    double maxVolts = 5.0;
    double clockPerBeat = 4.0;    // pulses per quarter note (4 = 1/16ths)
    double clockWidth = 0.5;      // fraction of each clock period that is high
    LfoShape lfoShape = LfoShape::Sine;
    double lfoBeats = 4.0;        // quarter notes per LFO cycle
    double testVolts = 0.0;
    double calOffsetVolts = 0.0;  // calibration: out = v * scale + offset
    double calScale = 1.0;
};

struct VoiceSettings {
    int zeroVoltNote = 36;        // MIDI note that outputs 0 V (36 = C1 in Logic)
    double bendRangeSemis = 2.0;
    double glideMs = 0.0;
    bool legato = false;          // true: overlapping notes don't retrigger
    double triggerMs = 5.0;       // Trigger and Reset pulse length
    double gateVolts = 5.0;       // high level for Gate/Trigger/Clock/Run/Reset
    double retriggerGapMs = 1.0;  // gate drop between overlapping notes
};

struct Transport {
    bool playing = false;
    bool hasPosition = false;     // host supplied a valid ppq position
    double bpm = 120.0;
    double ppqAtBlockStart = 0.0;
};

class Engine {
public:
    void prepare(double sampleRate, int maxDelaySamples);
    void reset();

    void setVoiceSettings(const VoiceSettings& s) { voice_ = s; }
    void setChannelSettings(int ch, const ChannelSettings& s) { channels_[(size_t) ch] = s; }
    // Positive delay shifts all output later; clamped to maxDelaySamples.
    void setOutputDelaySamples(int samples);

    void noteOn(int note, int velocity);
    void noteOff(int note);
    void pitchBend(int value14);  // 0..16383, 8192 = centre
    void controlChange(int cc, int value);
    void allNotesOff();

    // Call once per host block, before any render() for that block.
    void beginBlock(const Transport& t, int numSamples);
    // Render block-relative sample range [start, end). Either pointer may be null.
    void render(float* left, float* right, int start, int end);

    // Introspection for tests / UI.
    double currentPitchVolts() const { return pitchVolts_; }
    bool gateIsHigh() const;
    int currentNote() const { return held_.empty() ? lastNote_ : held_.back(); }

    static float voltsToSample(double volts, const ChannelSettings& cal);
    static double lfoValue(LfoShape shape, double phase01); // returns -1..1

private:
    double targetPitchVolts() const;
    double channelVolts(int ch, double ppq, bool positionValid);
    int msToSamples(double ms) const;

    double sampleRate_ = 48000.0;
    VoiceSettings voice_;
    std::array<ChannelSettings, kNumChannels> channels_{};

    // Mono voice state
    std::vector<int> held_;       // note stack, back = most recent
    int lastNote_ = 36;
    int lastVelocity_ = 0;
    double bend_ = 0.0;           // -1..1
    double pitchVolts_ = 0.0;
    bool pitchInitialised_ = false;
    int gapRemaining_ = 0;
    int triggerRemaining_ = 0;

    // CCs (0..1) and per-channel smoothing
    std::array<double, 128> cc_{};
    std::array<double, kNumChannels> ccSmoothed_{};
    std::array<double, kNumChannels> lfoFreePhase_{};

    // Transport
    Transport transport_;
    bool wasPlaying_ = false;
    double expectedPpq_ = 0.0;
    int resetRemaining_ = 0;
    double beatsPerSample_ = 0.0;

    // Output delay
    std::array<std::vector<float>, kNumChannels> delayBuf_;
    size_t delayWrite_ = 0;
    int delaySamples_ = 0;
};

} // namespace es9cv
