#include "es9cv/Engine.h"

#include <algorithm>
#include <cmath>

namespace es9cv {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kCcSmoothMs = 2.0;

double frac(double x) { return x - std::floor(x); }
} // namespace

void Engine::prepare(double sampleRate, int maxDelaySamples)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    for (auto& b : delayBuf_)
        b.assign((size_t) std::max(1, maxDelaySamples + 1), 0.0f);
    delaySamples_ = std::min(delaySamples_, maxDelaySamples);
    reset();
}

void Engine::reset()
{
    held_.clear();
    gapRemaining_ = triggerRemaining_ = resetRemaining_ = 0;
    bend_ = 0.0;
    pitchInitialised_ = false;
    wasPlaying_ = false;
    lfoFreePhase_.fill(0.0);
    for (auto& b : delayBuf_)
        std::fill(b.begin(), b.end(), 0.0f);
    delayWrite_ = 0;
}

void Engine::setOutputDelaySamples(int samples)
{
    const int maxDelay = (int) delayBuf_[0].size() - 1;
    delaySamples_ = std::clamp(samples, 0, std::max(0, maxDelay));
}

int Engine::msToSamples(double ms) const
{
    return std::max(0, (int) std::lround(ms * sampleRate_ / 1000.0));
}

// ---------------------------------------------------------------- events

void Engine::noteOn(int note, int velocity)
{
    if (velocity <= 0) { noteOff(note); return; }

    const bool wasHeld = !held_.empty();
    held_.erase(std::remove(held_.begin(), held_.end(), note), held_.end());
    held_.push_back(note);
    lastNote_ = note;
    lastVelocity_ = velocity;

    if (!pitchInitialised_) {
        pitchVolts_ = targetPitchVolts();
        pitchInitialised_ = true;
    }

    if (!(voice_.legato && wasHeld)) {
        triggerRemaining_ = msToSamples(voice_.triggerMs);
        if (wasHeld)
            gapRemaining_ = std::max(1, msToSamples(voice_.retriggerGapMs));
    }
}

void Engine::noteOff(int note)
{
    held_.erase(std::remove(held_.begin(), held_.end(), note), held_.end());
    if (!held_.empty())
        lastNote_ = held_.back();  // fall back to previous held note, no retrigger
}

void Engine::pitchBend(int value14)
{
    bend_ = std::clamp((value14 - 8192) / 8192.0, -1.0, 1.0);
}

void Engine::controlChange(int cc, int value)
{
    if (cc < 0 || cc > 127) return;
    if (cc == 123 || cc == 120) { allNotesOff(); return; }
    cc_[(size_t) cc] = std::clamp(value, 0, 127) / 127.0;
}

void Engine::allNotesOff()
{
    held_.clear();
    gapRemaining_ = 0;
    triggerRemaining_ = 0;
}

bool Engine::gateIsHigh() const
{
    return !held_.empty() && gapRemaining_ == 0;
}

double Engine::targetPitchVolts() const
{
    const double semis = (lastNote_ - voice_.zeroVoltNote) + bend_ * voice_.bendRangeSemis;
    return semis / 12.0;
}

// ---------------------------------------------------------------- transport

void Engine::beginBlock(const Transport& t, int numSamples)
{
    transport_ = t;
    const double bpm = t.bpm > 0.0 ? t.bpm : 120.0;
    beatsPerSample_ = bpm / 60.0 / sampleRate_;

    const bool positioned = t.playing && t.hasPosition;
    if (t.playing && !wasPlaying_) {
        resetRemaining_ = msToSamples(voice_.triggerMs);
    } else if (positioned && wasPlaying_) {
        // Loop or locate backwards while playing -> reset pulse.
        if (t.ppqAtBlockStart < expectedPpq_ - 1.0e-3)
            resetRemaining_ = msToSamples(voice_.triggerMs);
    }
    wasPlaying_ = t.playing;
    expectedPpq_ = t.ppqAtBlockStart + numSamples * beatsPerSample_;
}

// ---------------------------------------------------------------- render

double Engine::lfoValue(LfoShape shape, double p)
{
    p = frac(p);
    switch (shape) {
        case LfoShape::Sine:     return std::sin(2.0 * kPi * p);
        case LfoShape::Triangle: return p < 0.5 ? 4.0 * p - 1.0 : 3.0 - 4.0 * p;
        case LfoShape::SawUp:    return 2.0 * p - 1.0;
        case LfoShape::SawDown:  return 1.0 - 2.0 * p;
        case LfoShape::Square:   return p < 0.5 ? 1.0 : -1.0;
        default:                 return 0.0;
    }
}

float Engine::voltsToSample(double volts, const ChannelSettings& cal)
{
    const double v = volts * cal.calScale + cal.calOffsetVolts;
    return (float) std::clamp(v / kFullScaleVolts, -1.0, 1.0);
}

double Engine::channelVolts(int ch, double ppq, bool positionValid)
{
    const auto& c = channels_[(size_t) ch];
    const double hi = voice_.gateVolts;
    const bool running = transport_.playing;

    switch (c.function) {
        case Function::Off:         return 0.0;
        case Function::Pitch:       return pitchVolts_;
        case Function::Gate:        return gateIsHigh() ? hi : 0.0;
        case Function::Trigger:     return triggerRemaining_ > 0 ? hi : 0.0;
        case Function::Velocity:
            return c.minVolts + (c.maxVolts - c.minVolts) * (lastVelocity_ / 127.0);
        case Function::CC:
            return c.minVolts + (c.maxVolts - c.minVolts) * ccSmoothed_[(size_t) ch];
        case Function::Clock:
            if (!running || !positionValid || c.clockPerBeat <= 0.0) return 0.0;
            return frac(ppq * c.clockPerBeat + 1.0e-9) < c.clockWidth ? hi : 0.0;
        case Function::Run:         return running ? hi : 0.0;
        case Function::Reset:       return resetRemaining_ > 0 ? hi : 0.0;
        case Function::LFO: {
            const double beats = c.lfoBeats > 0.0 ? c.lfoBeats : 1.0;
            const double phase = (running && positionValid) ? ppq / beats
                                                            : lfoFreePhase_[(size_t) ch];
            const double uni = 0.5 * (lfoValue(c.lfoShape, phase) + 1.0);
            return c.minVolts + (c.maxVolts - c.minVolts) * uni;
        }
        case Function::TestVoltage: return c.testVolts;
        default:                    return 0.0;
    }
}

void Engine::render(float* left, float* right, int start, int end)
{
    float* outs[kNumChannels] = { left, right };
    const bool positionValid = transport_.hasPosition;

    const double glideSamples = voice_.glideMs * sampleRate_ / 1000.0;
    const double glideCoeff = glideSamples > 1.0 ? std::exp(-1.0 / glideSamples) : 0.0;
    const double ccCoeff = std::exp(-1.0 / std::max(1.0, kCcSmoothMs * sampleRate_ / 1000.0));
    const size_t bufLen = delayBuf_[0].size();

    for (int i = start; i < end; ++i) {
        // Voice
        const double target = targetPitchVolts();
        if (!pitchInitialised_) { pitchVolts_ = target; pitchInitialised_ = true; }
        pitchVolts_ = target + (pitchVolts_ - target) * glideCoeff;

        const double ppq = transport_.ppqAtBlockStart + i * beatsPerSample_;

        for (int ch = 0; ch < kNumChannels; ++ch) {
            const auto& c = channels_[(size_t) ch];
            auto& s = ccSmoothed_[(size_t) ch];
            const double ccTarget = cc_[(size_t) std::clamp(c.ccNumber, 0, 127)];
            s = ccTarget + (s - ccTarget) * ccCoeff;

            const float sample = voltsToSample(channelVolts(ch, ppq, positionValid), c);

            if (bufLen > 1) {
                auto& buf = delayBuf_[(size_t) ch];
                buf[delayWrite_] = sample;
                const size_t readIdx = (delayWrite_ + bufLen - (size_t) delaySamples_) % bufLen;
                if (outs[ch]) outs[ch][i] = buf[readIdx];
            } else if (outs[ch]) {
                outs[ch][i] = sample;
            }

            if (!transport_.playing || !positionValid) {
                const double beats = c.lfoBeats > 0.0 ? c.lfoBeats : 1.0;
                lfoFreePhase_[(size_t) ch] = frac(lfoFreePhase_[(size_t) ch] + beatsPerSample_ / beats);
            }
        }
        if (bufLen > 1) delayWrite_ = (delayWrite_ + 1) % bufLen;

        // Countdowns
        if (gapRemaining_ > 0) --gapRemaining_;
        if (triggerRemaining_ > 0) --triggerRemaining_;
        if (resetRemaining_ > 0) --resetRemaining_;
    }
}

} // namespace es9cv
