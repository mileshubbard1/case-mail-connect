# ES9 CV — Logic Pro → Expert Sleepers ES-9 CV/Gate

A personal Audio Unit instrument for Logic Pro. It turns MIDI from a Logic
track into DC voltages on two ES-9 output jacks: pitch, gate, trigger,
velocity, CC, clock, run, reset, an LFO or a fixed test voltage.

**Setup (layout B):**

| ES-9 outs | Logic track | Plugin program |
|---|---|---|
| 1-2 | Voice 1 | Voice: Pitch + Gate |
| 3-4 | Voice 2 | Voice: Pitch + Gate |
| 5-6 | Voice 3 | Voice: Pitch + Gate |
| 7-8 | Utility | Utility: Clock + LFO (or Clock + Reset) |

Each instance outputs a stereo pair: **L jack = left channel, R jack = right
channel**. The ES-9 turns a sample value of 1.0 into +10 V, so 1 V/oct pitch
is just a value of 0.1 per octave.

---

## 1. Build (macOS)

You need Xcode (or the Command Line Tools) and CMake 3.22 or newer (`brew install cmake`).

```sh
cd es9-cv
cmake -B build -G Xcode            # first run downloads JUCE 8.0.4
cmake --build build --config Release
```

The build copies `ES9 CV.component` to `~/Library/Audio/Plug-Ins/Components/`.
Check it:

```sh
auval -v aumu E9cv Escv            # should end with "AU VALIDATION SUCCEEDED"
```

If Logic doesn't list it, quit Logic and run `killall -9 AudioComponentRegistrar`,
then reopen Logic. Also check **Logic → Settings → Plug-in Manager**.

Unit tests for the CV engine (they run on any OS, no JUCE needed):

```sh
cmake -B build-tests -DES9CV_BUILD_PLUGIN=OFF && cmake --build build-tests && ctest --test-dir build-tests
```

## 2. ES-9 and Logic setup (do this once)

1. **ES-9 Config Tool:** route USB outputs 1–8 straight to physical
   outputs 1–8, with no mixer gain in the path.
2. **Logic → Settings → Audio → Output device:** ES-9. You monitor through the
   ES-9's line/headphone outs, or through your modular's mixer.
3. **Logic → Settings → Audio → I/O Assignments / I/O Labels:** label outputs
   1-2 to 7-8 as "CV V1" … "CV Util".
4. Create **4 Software Instrument tracks** and insert *AU Instruments → ES9CV → ES9 CV* on each.
5. Set each track's **Output** slot to its pair (Output 1-2, 3-4, 5-6, 7-8).
6. **Unity gain is mandatory.** Leave the track fader at 0.0 dB and the pan
   centred (stereo balance at centre is unity), with no plugins after the
   instrument. Do the same on the **Output 1-2 … 7-8 channel strips** in the
   Mixer: 0.0 dB, no plugins.
7. Pick each instance's program from the preset menu at the top of the plugin
   window (table above).
8. Save the project as a template (**File → Save as Template**) so you
   never have to repeat this.

> Make sure Logic's stereo output (your monitoring) isn't also routed to
> Output 1-2, or your music will come out of a CV jack.

## 3. Phase 0 — DC test (before plugging into any modules)

Check that a known voltage really appears on every jack.

1. Load the program **DC Test / Calibrate** on all four instances.
2. Set **L Test Voltage** and **R Test Voltage** to `1.000 V`. Press Play once
   so Logic processes the tracks (see the note on inactive tracks below).
3. Measure every jack with a multimeter, from tip to sleeve: Outs 1–8 should each read about 1.00 V.
4. Repeat at `5.000 V` and `-5.000 V`.

If a reading is way off (for example 0.7 V), something in the chain isn't at
unity gain: a fader, pan, the Output strip, or the ES-9 Config Tool routing. Fix that first.

## 4. Calibration (per jack)

Pitch accuracy depends on this. Calibration is stored inside each instance,
so calibrate each instance on the jack it will actually drive.

1. With **Test Voltage** mode on the jack, set Test Voltage to `0.000 V`.
   Adjust **Cal Offset** until the meter reads 0.000 V.
2. Set Test Voltage to `5.000 V`. Adjust **Cal Scale** until the meter reads 5.000 V.
3. Repeat steps 1–2 once, because the two settings interact slightly.
4. Switch back to the Voice program. Selecting a program only changes the jack
   functions; your calibration is kept.

Better still, tune by ear: play a VCO at `0V Note` and at 4 octaves above it
with a tuner, and adjust Cal Scale until both octaves read in tune.

## 5. Parameters

**Voice (shared by both jacks)**
- **0V Note:** the MIDI note that outputs 0 V. The default is 36, which Logic calls C1.
- **Bend Range** (semitones), **Glide** (ms), **Legato** (overlapping notes
  don't retrigger the gate).
- **Trigger Length:** pulse width for Trigger and Reset.
- **Gate Level:** the high voltage for Gate, Trigger, Clock, Run and Reset (default 5 V).
- **Timing Offset:** positive values delay the CV. Negative values make Logic
  play the track earlier, because the plugin reports latency to Logic's
  delay compensation. If the change doesn't seem to take effect, stop and
  restart the transport.

**Per jack (L / R)**
- **Function:** Off, Pitch, Gate, Trigger, Velocity, CC, Clock, Run, Reset, LFO or Test Voltage.
- **Range Min / Max:** the output range for Velocity, CC and LFO.
- **CC Number:** which CC the CC mode follows (1 = mod wheel). CCs are smoothed over about 2 ms.
- **Clock Division** (1/1 … 1/32, 24 or 48 PPQN) and **Clock Width**: the
  clock runs only while the transport plays, locked to Logic's song position.
- **LFO Shape** and **LFO Rate** (8 bars … 1/16): locked to the song position
  while playing, free-running at the session tempo when stopped.
- **Test Voltage**, **Cal Offset**, **Cal Scale**.

**Behaviour notes**
- Pitch keeps its last value after you release a note, so release tails stay in tune.
- Last-note priority: releasing the top note drops back to a still-held note without retriggering.
- Reset fires when Play starts and whenever the playhead jumps backwards, such as a cycle loop.
- MIDI is handled to the exact sample (the block is split at each event).
- Every output is clamped to ±10 V.

## 6. Known quirks and things to check

- **Inactive tracks:** Logic may stop processing a track that isn't selected
  or playing anything. This mostly affects the Test Voltage and LFO modes when
  the transport is stopped. While playing, the instrument is always processed.
- **Bouncing:** CV is just audio to Logic, so an offline bounce does nothing
  useful. Use real-time playback.
- **Latency:** CV leaves through the same output path as Logic's audio, so it
  lines up with Logic's playback. Audio coming *back* from the modular is
  delayed by the round trip, so use Timing Offset or Logic's recording delay setting to compensate.

## Layout

```
es9-cv/
  core/      framework-free CV engine (Engine.h / Engine.cpp)
  tests/     dependency-free unit tests for the engine
  plugin/    JUCE Audio Unit wrapper (parameters, MIDI, transport)
  docs/      SCOPE.md: project scope and decisions
```
