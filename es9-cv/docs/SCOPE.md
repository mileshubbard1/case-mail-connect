# Scope and decisions

**Goal:** play and sequence Eurorack from Logic Pro through the 8 DC-coupled
outputs of an Expert Sleepers ES-9. This is for personal use only, with no expanders.

## Decisions
- **Layout B:** 3 pitch+gate voices on Outs 1-2, 3-4 and 5-6, plus a utility
  pair on 7-8 (clock + LFO, or clock + reset).
- **One plugin, one stereo pair per instance.** Each jack has a mode switch. A
  track's output is set to the ES-9 pair. This avoids Logic's multi-output
  instrument routing; a stereo strip's pan at centre is unity gain.
- **AU instrument (`aumu`), built with JUCE 8.** Logic's MIDI FX plugins can't output audio.
- **The CV engine is free of JUCE and unit-tested**, so voltage and timing logic
  can be checked without a Mac.
- **Calibration is manual:** offset + scale per jack, set with a multimeter or a tuner.
- **The UI is JUCE's generic parameter editor.** Logic's own controls view also works.

## Out of scope (for now)
Polyphonic voice allocation across outputs, ES-5/ES-6 support, auto-calibration
through the ES-9 inputs, a custom UI, signing/notarization and an installer.

## Phases
| Phase | Deliverable | Status |
|---|---|---|
| 0 | DC test on all 8 jacks, unity-gain check, try Silent Way | Ready to run (README §3) |
| 1 | Pitch + gate voice, calibration | Code complete, needs testing on hardware |
| 2 | Channel modes, clock/LFO, timing offset, Logic template | Code complete except the template; needs testing on hardware |

## Open risks to verify on the Mac
1. Unity gain through Logic's Output channel strips (Phase 0).
2. Logic's inactive-track processing when the transport is stopped.
3. Whether a timing offset change is picked up while playing, or only after stop/start.
4. Slight drift in ES-9 pitch tracking across 5+ octaves. If offset + scale
   isn't enough, add a per-octave calibration table.
