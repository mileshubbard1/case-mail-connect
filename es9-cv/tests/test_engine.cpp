// Dependency-free unit tests for the ES9 CV engine. Run via ctest.
#include "es9cv/Engine.h"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace es9cv;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)
#define CHECK_NEAR(a, b, eps) do { double _a = (a), _b = (b); if (std::fabs(_a - _b) > (eps)) { \
    std::printf("FAIL %s:%d  %s = %.6f, expected %.6f\n", __FILE__, __LINE__, #a, _a, _b); ++failures; } } while (0)

constexpr double SR = 48000.0;
constexpr double kV = 1.0 / kFullScaleVolts; // one volt as a sample value

struct Rig {
    Engine e;
    std::vector<float> l, r;
    Rig(Function lf = Function::Pitch, Function rf = Function::Gate, int maxDelay = 0) {
        e.prepare(SR, maxDelay);
        ChannelSettings a, b; a.function = lf; b.function = rf;
        e.setChannelSettings(0, a); e.setChannelSettings(1, b);
    }
    void run(int n, Transport t = {}) {
        l.assign((size_t) n, -99.f); r.assign((size_t) n, -99.f);
        e.beginBlock(t, n);
        e.render(l.data(), r.data(), 0, n);
    }
};

static void testPitchIs1VPerOctave()
{
    Rig rig;
    rig.e.noteOn(36, 100); rig.run(16);
    CHECK_NEAR(rig.l.back(), 0.0, 1e-6);
    CHECK_NEAR(rig.r.back(), 5.0 * kV, 1e-6);   // 5 V gate
    rig.e.noteOn(48, 100); rig.run(256);
    CHECK_NEAR(rig.l.back(), 1.0 * kV, 1e-6);   // +1 octave = 1 V
    rig.e.noteOn(24, 100); rig.run(256);
    CHECK_NEAR(rig.l.back(), -1.0 * kV, 1e-6);
    rig.e.noteOn(43, 100); rig.run(256);
    CHECK_NEAR(rig.l.back(), 7.0 / 12.0 * kV, 1e-6);
}

static void testLastNotePriorityAndRelease()
{
    Rig rig;
    rig.e.noteOn(36, 100); rig.e.noteOn(48, 100); rig.run(256);
    CHECK_NEAR(rig.l.back(), 1.0 * kV, 1e-6);
    rig.e.noteOff(48); rig.run(256);
    CHECK_NEAR(rig.l.back(), 0.0, 1e-6);        // back to held note
    CHECK(rig.e.gateIsHigh());
    rig.e.noteOff(36); rig.run(16);
    CHECK_NEAR(rig.r.back(), 0.0, 1e-6);        // gate low
    CHECK_NEAR(rig.l.back(), 0.0, 1e-6);        // pitch holds for release tails
}

static void testRetriggerGapAndLegato()
{
    Rig rig;
    rig.e.noteOn(36, 100); rig.run(256);
    rig.e.noteOn(40, 100); rig.run(256);
    CHECK_NEAR(rig.r[0], 0.0, 1e-6);            // gap at start of overlap
    CHECK_NEAR(rig.r.back(), 5.0 * kV, 1e-6);

    Rig leg;
    VoiceSettings v; v.legato = true; leg.e.setVoiceSettings(v);
    leg.e.noteOn(36, 100); leg.run(256);
    leg.e.noteOn(40, 100); leg.run(256);
    CHECK_NEAR(leg.r[0], 5.0 * kV, 1e-6);       // no gap in legato
}

static void testSampleAccurateSplit()
{
    Rig rig(Function::Pitch, Function::Gate);
    rig.l.assign(100, 0.f); rig.r.assign(100, 0.f);
    rig.e.beginBlock({}, 100);
    rig.e.render(rig.l.data(), rig.r.data(), 0, 37);
    rig.e.noteOn(36, 100);
    rig.e.render(rig.l.data(), rig.r.data(), 37, 100);
    CHECK_NEAR(rig.r[36], 0.0, 1e-6);
    CHECK_NEAR(rig.r[37], 5.0 * kV, 1e-6);
}

static void testTriggerLength()
{
    Rig rig(Function::Trigger, Function::Off);
    VoiceSettings v; v.triggerMs = 1.0; rig.e.setVoiceSettings(v); // 48 samples
    rig.e.noteOn(60, 100); rig.run(100);
    CHECK_NEAR(rig.l[47], 5.0 * kV, 1e-6);
    CHECK_NEAR(rig.l[48], 0.0, 1e-6);
}

static void testPitchBend()
{
    Rig rig;
    rig.e.noteOn(36, 100); rig.e.pitchBend(16383); rig.run(16);
    CHECK_NEAR(rig.l.back(), 2.0 / 12.0 * kV, 1e-4); // +2 semis default range
}

static void testGlide()
{
    Rig rig;
    VoiceSettings v; v.glideMs = 50.0; rig.e.setVoiceSettings(v);
    rig.e.noteOn(36, 100); rig.run(16);
    CHECK_NEAR(rig.l.back(), 0.0, 1e-6);         // first note jumps
    rig.e.noteOn(48, 100); rig.run(480);         // 10 ms
    CHECK(rig.l.back() > 0.0f && rig.l.back() < 1.0 * kV);
    rig.run(48000);
    CHECK_NEAR(rig.l.back(), 1.0 * kV, 1e-5);
}

static void testCalibrationAndClamp()
{
    ChannelSettings c; c.calScale = 1.01; c.calOffsetVolts = -0.02;
    CHECK_NEAR(Engine::voltsToSample(2.0, c), (2.0 * 1.01 - 0.02) * kV, 1e-6);
    ChannelSettings plain;
    CHECK_NEAR(Engine::voltsToSample(25.0, plain), 1.0, 1e-9);
    CHECK_NEAR(Engine::voltsToSample(-25.0, plain), -1.0, 1e-9);
}

static void testTestVoltage()
{
    Rig rig(Function::TestVoltage, Function::TestVoltage);
    ChannelSettings a; a.function = Function::TestVoltage; a.testVolts = 3.0;
    ChannelSettings b; b.function = Function::TestVoltage; b.testVolts = -5.0;
    rig.e.setChannelSettings(0, a); rig.e.setChannelSettings(1, b);
    rig.run(8);
    CHECK_NEAR(rig.l.back(), 0.3, 1e-6);
    CHECK_NEAR(rig.r.back(), -0.5, 1e-6);
}

static void testClockAndReset()
{
    Rig rig(Function::Clock, Function::Reset);
    ChannelSettings clk; clk.function = Function::Clock; clk.clockPerBeat = 4.0; clk.clockWidth = 0.5;
    rig.e.setChannelSettings(0, clk);
    Transport t; t.playing = true; t.hasPosition = true; t.bpm = 120.0; t.ppqAtBlockStart = 0.0;
    // 120 bpm, 1/16 clock -> period 0.125 s = 6000 samples, high for 3000
    rig.run(12000, t);
    CHECK_NEAR(rig.l[0], 5.0 * kV, 1e-6);
    CHECK_NEAR(rig.l[2999], 5.0 * kV, 1e-6);
    CHECK_NEAR(rig.l[3001], 0.0, 1e-6);
    CHECK_NEAR(rig.l[6001], 5.0 * kV, 1e-6);
    CHECK_NEAR(rig.r[0], 5.0 * kV, 1e-6);          // reset pulse on start
    CHECK_NEAR(rig.r[1000], 0.0, 1e-6);

    // Continue normally: no reset
    t.ppqAtBlockStart = 12000 * (120.0 / 60.0 / SR);
    rig.run(100, t);
    CHECK_NEAR(rig.r[0], 0.0, 1e-6);
    // Loop back -> reset
    t.ppqAtBlockStart = 0.0;
    rig.run(100, t);
    CHECK_NEAR(rig.r[0], 5.0 * kV, 1e-6);
    // Stopped -> clock low
    t.playing = false;
    rig.run(100, t);
    CHECK_NEAR(rig.l[0], 0.0, 1e-6);
}

static void testLfoSyncedToPpq()
{
    Rig rig(Function::LFO, Function::Off);
    ChannelSettings c; c.function = Function::LFO; c.lfoShape = LfoShape::SawUp;
    c.lfoBeats = 1.0; c.minVolts = 0.0; c.maxVolts = 10.0;
    rig.e.setChannelSettings(0, c);
    Transport t; t.playing = true; t.hasPosition = true; t.bpm = 120.0; t.ppqAtBlockStart = 0.25;
    rig.run(4, t);
    CHECK_NEAR(rig.l[0], 0.25, 1e-3);              // quarter of the way up a 0..10 V saw
}

static void testCcSmoothedToRange()
{
    Rig rig(Function::CC, Function::Velocity);
    ChannelSettings c; c.function = Function::CC; c.ccNumber = 74; c.minVolts = 0; c.maxVolts = 8;
    ChannelSettings v; v.function = Function::Velocity; v.minVolts = 0; v.maxVolts = 5;
    rig.e.setChannelSettings(0, c); rig.e.setChannelSettings(1, v);
    rig.e.controlChange(74, 127);
    rig.e.noteOn(60, 127);
    rig.run(4800);
    CHECK_NEAR(rig.l.back(), 0.8, 1e-4);
    CHECK_NEAR(rig.r.back(), 0.5, 1e-6);
}

static void testOutputDelay()
{
    Rig rig(Function::Gate, Function::Off, 1000);
    rig.e.setOutputDelaySamples(10);
    rig.e.noteOn(60, 100); rig.run(20);
    CHECK_NEAR(rig.l[9], 0.0, 1e-6);
    CHECK_NEAR(rig.l[10], 5.0 * kV, 1e-6);
}

int main()
{
    testPitchIs1VPerOctave();
    testLastNotePriorityAndRelease();
    testRetriggerGapAndLegato();
    testSampleAccurateSplit();
    testTriggerLength();
    testPitchBend();
    testGlide();
    testCalibrationAndClamp();
    testTestVoltage();
    testClockAndReset();
    testLfoSyncedToPpq();
    testCcSmoothedToRange();
    testOutputDelay();
    if (failures == 0) std::printf("All tests passed\n");
    return failures == 0 ? 0 : 1;
}
