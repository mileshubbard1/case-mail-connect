#include "PluginProcessor.h"

#include <array>
#include <cmath>

using es9cv::Function;
using es9cv::LfoShape;

namespace {

constexpr double kMaxDelayMs = 50.0;

const juce::StringArray kFunctionNames {
    "Off", "Pitch (1V/oct)", "Gate", "Trigger", "Velocity", "CC", "Clock",
    "Run", "Reset", "LFO", "Test Voltage"
};

struct Division { const char* name; double perBeat; };
const std::array<Division, 10> kClockDivs {{
    { "1/1", 0.25 }, { "1/2", 0.5 }, { "1/4", 1.0 }, { "1/8", 2.0 }, { "1/8T", 3.0 },
    { "1/16", 4.0 }, { "1/16T", 6.0 }, { "1/32", 8.0 }, { "24 PPQN", 24.0 }, { "48 PPQN", 48.0 }
}};
constexpr int kDefaultClockDiv = 5; // 1/16

const juce::StringArray kLfoShapes { "Sine", "Triangle", "Saw Up", "Saw Down", "Square" };

struct LfoRate { const char* name; double beats; };
const std::array<LfoRate, 8> kLfoRates {{
    { "8 bars", 32.0 }, { "4 bars", 16.0 }, { "2 bars", 8.0 }, { "1 bar", 4.0 },
    { "1/2", 2.0 }, { "1/4", 1.0 }, { "1/8", 0.5 }, { "1/16", 0.25 }
}};
constexpr int kDefaultLfoRate = 3; // 1 bar

const char* kChPrefix[es9cv::kNumChannels] = { "l_", "r_" };
const char* kChLabel[es9cv::kNumChannels]  = { "L ", "R " };

juce::String chId(int ch, const char* id) { return juce::String(kChPrefix[ch]) + id; }

// Factory programs: only the jack functions change; calibration is kept.
struct Program { const char* name; Function left; Function right; };
const std::array<Program, 6> kPrograms {{
    { "Voice: Pitch + Gate",         Function::Pitch,       Function::Gate },
    { "Voice: Pitch + Trigger",      Function::Pitch,       Function::Trigger },
    { "Utility: Clock + LFO",        Function::Clock,       Function::LFO },
    { "Utility: Clock + Reset",      Function::Clock,       Function::Reset },
    { "Mod: Velocity + CC",          Function::Velocity,    Function::CC },
    { "DC Test / Calibrate",         Function::TestVoltage, Function::TestVoltage },
}};

template <typename Array>
juce::StringArray namesOf(const Array& arr)
{
    juce::StringArray s;
    for (const auto& item : arr) s.add(item.name);
    return s;
}

} // namespace

ES9CVProcessor::ES9CVProcessor()
    : AudioProcessor(BusesProperties().withOutput("CV", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "ES9CV", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ES9CVProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;
    auto volts = [](float lo, float hi, float step) { return NormalisableRange<float>(lo, hi, step); };

    // ---- Voice (shared by both jacks)
    auto voice = std::make_unique<AudioProcessorParameterGroup>("voice", "Voice", " | ");
    voice->addChild(std::make_unique<AudioParameterInt>(ParameterID { "zeroNote", 1 }, "0V Note", 0, 127, 36));
    voice->addChild(std::make_unique<AudioParameterFloat>(ParameterID { "bendRange", 1 }, "Bend Range",
        NormalisableRange<float>(0.f, 24.f, 1.f), 2.f, AudioParameterFloatAttributes().withLabel("st")));
    voice->addChild(std::make_unique<AudioParameterFloat>(ParameterID { "glideMs", 1 }, "Glide",
        NormalisableRange<float>(0.f, 2000.f, 0.1f, 0.3f), 0.f, AudioParameterFloatAttributes().withLabel("ms")));
    voice->addChild(std::make_unique<AudioParameterBool>(ParameterID { "legato", 1 }, "Legato", false));
    voice->addChild(std::make_unique<AudioParameterFloat>(ParameterID { "triggerMs", 1 }, "Trigger Length",
        NormalisableRange<float>(1.f, 50.f, 0.1f), 5.f, AudioParameterFloatAttributes().withLabel("ms")));
    voice->addChild(std::make_unique<AudioParameterFloat>(ParameterID { "gateVolts", 1 }, "Gate Level",
        volts(1.f, 10.f, 0.1f), 5.f, AudioParameterFloatAttributes().withLabel("V")));
    voice->addChild(std::make_unique<AudioParameterFloat>(ParameterID { "offsetMs", 1 }, "Timing Offset",
        NormalisableRange<float>((float) -kMaxDelayMs, (float) kMaxDelayMs, 0.1f), 0.f,
        AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::move(voice));

    // ---- Per jack
    for (int ch = 0; ch < es9cv::kNumChannels; ++ch) {
        const String lbl = kChLabel[ch];
        auto g = std::make_unique<AudioProcessorParameterGroup>(kChPrefix[ch], ch == 0 ? "Left Jack" : "Right Jack", " | ");
        g->addChild(std::make_unique<AudioParameterChoice>(ParameterID { chId(ch, "function"), 1 }, lbl + "Function",
            kFunctionNames, ch == 0 ? (int) Function::Pitch : (int) Function::Gate));
        g->addChild(std::make_unique<AudioParameterInt>(ParameterID { chId(ch, "cc"), 1 }, lbl + "CC Number", 0, 127, 1));
        g->addChild(std::make_unique<AudioParameterFloat>(ParameterID { chId(ch, "minV"), 1 }, lbl + "Range Min",
            volts(-10.f, 10.f, 0.01f), 0.f, AudioParameterFloatAttributes().withLabel("V")));
        g->addChild(std::make_unique<AudioParameterFloat>(ParameterID { chId(ch, "maxV"), 1 }, lbl + "Range Max",
            volts(-10.f, 10.f, 0.01f), 5.f, AudioParameterFloatAttributes().withLabel("V")));
        g->addChild(std::make_unique<AudioParameterChoice>(ParameterID { chId(ch, "clockDiv"), 1 }, lbl + "Clock Division",
            namesOf(kClockDivs), kDefaultClockDiv));
        g->addChild(std::make_unique<AudioParameterFloat>(ParameterID { chId(ch, "clockWidth"), 1 }, lbl + "Clock Width",
            NormalisableRange<float>(5.f, 95.f, 1.f), 50.f, AudioParameterFloatAttributes().withLabel("%")));
        g->addChild(std::make_unique<AudioParameterChoice>(ParameterID { chId(ch, "lfoShape"), 1 }, lbl + "LFO Shape",
            kLfoShapes, 0));
        g->addChild(std::make_unique<AudioParameterChoice>(ParameterID { chId(ch, "lfoRate"), 1 }, lbl + "LFO Rate",
            namesOf(kLfoRates), kDefaultLfoRate));
        g->addChild(std::make_unique<AudioParameterFloat>(ParameterID { chId(ch, "testV"), 1 }, lbl + "Test Voltage",
            volts(-10.f, 10.f, 0.001f), 0.f, AudioParameterFloatAttributes().withLabel("V")));
        g->addChild(std::make_unique<AudioParameterFloat>(ParameterID { chId(ch, "calOffset"), 1 }, lbl + "Cal Offset",
            volts(-0.5f, 0.5f, 0.0001f), 0.f, AudioParameterFloatAttributes().withLabel("V")));
        g->addChild(std::make_unique<AudioParameterFloat>(ParameterID { chId(ch, "calScale"), 1 }, lbl + "Cal Scale",
            NormalisableRange<float>(0.9f, 1.1f, 0.0001f), 1.f));
        layout.add(std::move(g));
    }
    return layout;
}

bool ES9CVProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet().isDisabled()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ES9CVProcessor::prepareToPlay(double sr, int)
{
    sampleRate = sr;
    engine.prepare(sr, (int) std::ceil(kMaxDelayMs * sr / 1000.0));
    pullSettings();
    reportedLatency = -1;
    updateLatency();
}

void ES9CVProcessor::pullSettings()
{
    auto raw = [this](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

    es9cv::VoiceSettings v;
    v.zeroVoltNote   = (int) raw("zeroNote");
    v.bendRangeSemis = raw("bendRange");
    v.glideMs        = raw("glideMs");
    v.legato         = raw("legato") > 0.5f;
    v.triggerMs      = raw("triggerMs");
    v.gateVolts      = raw("gateVolts");
    engine.setVoiceSettings(v);

    for (int ch = 0; ch < es9cv::kNumChannels; ++ch) {
        es9cv::ChannelSettings c;
        c.function       = (Function) juce::jlimit(0, (int) Function::Count - 1, (int) raw(chId(ch, "function")));
        c.ccNumber       = (int) raw(chId(ch, "cc"));
        c.minVolts       = raw(chId(ch, "minV"));
        c.maxVolts       = raw(chId(ch, "maxV"));
        c.clockPerBeat   = kClockDivs[(size_t) juce::jlimit(0, (int) kClockDivs.size() - 1, (int) raw(chId(ch, "clockDiv")))].perBeat;
        c.clockWidth     = raw(chId(ch, "clockWidth")) / 100.0;
        c.lfoShape       = (LfoShape) juce::jlimit(0, (int) LfoShape::Count - 1, (int) raw(chId(ch, "lfoShape")));
        c.lfoBeats       = kLfoRates[(size_t) juce::jlimit(0, (int) kLfoRates.size() - 1, (int) raw(chId(ch, "lfoRate")))].beats;
        c.testVolts      = raw(chId(ch, "testV"));
        c.calOffsetVolts = raw(chId(ch, "calOffset"));
        c.calScale       = raw(chId(ch, "calScale"));
        engine.setChannelSettings(ch, c);
    }
}

// Positive offset = CV later (internal delay). Negative offset = CV earlier:
// we report latency to Logic, whose delay compensation then plays this track early.
void ES9CVProcessor::updateLatency()
{
    const double ms = apvts.getRawParameterValue("offsetMs")->load();
    const int samples = (int) std::lround(std::abs(ms) * sampleRate / 1000.0);
    engine.setOutputDelaySamples(ms > 0.0 ? samples : 0);
    const int latency = ms < 0.0 ? samples : 0;
    if (latency != reportedLatency) {
        reportedLatency = latency;
        setLatencySamples(latency);
    }
}

void ES9CVProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    pullSettings();
    updateLatency();

    const int n = buffer.getNumSamples();
    buffer.clear();

    es9cv::Transport t;
    if (auto* head = getPlayHead()) {
        if (auto pos = head->getPosition()) {
            t.playing = pos->getIsPlaying();
            if (auto bpm = pos->getBpm()) t.bpm = *bpm;
            if (auto ppq = pos->getPpqPosition()) { t.ppqAtBlockStart = *ppq; t.hasPosition = true; }
        }
    }
    engine.beginBlock(t, n);

    float* left  = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    // Split the block at each MIDI event for sample-accurate timing.
    int pos = 0;
    for (const auto meta : midi) {
        const int at = juce::jlimit(0, n, meta.samplePosition);
        if (at > pos) { engine.render(left, right, pos, at); pos = at; }

        const auto msg = meta.getMessage();
        if (msg.isNoteOn())                 engine.noteOn(msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isNoteOff())           engine.noteOff(msg.getNoteNumber());
        else if (msg.isPitchWheel())        engine.pitchBend(msg.getPitchWheelValue());
        else if (msg.isController())        engine.controlChange(msg.getControllerNumber(), msg.getControllerValue());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine.allNotesOff();
    }
    if (pos < n) engine.render(left, right, pos, n);
}

int ES9CVProcessor::getNumPrograms() { return (int) kPrograms.size(); }

const juce::String ES9CVProcessor::getProgramName(int index)
{
    return juce::isPositiveAndBelow(index, (int) kPrograms.size()) ? kPrograms[(size_t) index].name : "";
}

void ES9CVProcessor::setCurrentProgram(int index)
{
    if (!juce::isPositiveAndBelow(index, (int) kPrograms.size())) return;
    currentProgram = index;
    const auto& p = kPrograms[(size_t) index];
    const Function fns[es9cv::kNumChannels] = { p.left, p.right };
    for (int ch = 0; ch < es9cv::kNumChannels; ++ch)
        if (auto* param = apvts.getParameter(chId(ch, "function")))
            param->setValueNotifyingHost(param->convertTo0to1((float) fns[ch]));
}

juce::AudioProcessorEditor* ES9CVProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void ES9CVProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void ES9CVProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ES9CVProcessor();
}
