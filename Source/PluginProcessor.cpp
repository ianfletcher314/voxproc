#include "PluginProcessor.h"
#include "PluginEditor.h"

VoxProcAudioProcessor::VoxProcAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Compressor parameters
    compThreshold = apvts.getRawParameterValue("compThreshold");
    compRatio = apvts.getRawParameterValue("compRatio");
    compAttack = apvts.getRawParameterValue("compAttack");
    compRelease = apvts.getRawParameterValue("compRelease");
    compMakeup = apvts.getRawParameterValue("compMakeup");
    compKnee = apvts.getRawParameterValue("compKnee");
    compAutoRelease = apvts.getRawParameterValue("compAutoRelease");
    compBypass = apvts.getRawParameterValue("compBypass");

    // De-esser parameters
    deessFrequency = apvts.getRawParameterValue("deessFrequency");
    deessThreshold = apvts.getRawParameterValue("deessThreshold");
    deessRange = apvts.getRawParameterValue("deessRange");
    deessMode = apvts.getRawParameterValue("deessMode");
    deessListen = apvts.getRawParameterValue("deessListen");
    deessBypass = apvts.getRawParameterValue("deessBypass");

    // EQ parameters
    eqHPFFreq = apvts.getRawParameterValue("eqHPFFreq");
    eqHPFSlope = apvts.getRawParameterValue("eqHPFSlope");
    eqLowShelfFreq = apvts.getRawParameterValue("eqLowShelfFreq");
    eqLowShelfGain = apvts.getRawParameterValue("eqLowShelfGain");
    eqLowMidFreq = apvts.getRawParameterValue("eqLowMidFreq");
    eqLowMidGain = apvts.getRawParameterValue("eqLowMidGain");
    eqLowMidQ = apvts.getRawParameterValue("eqLowMidQ");
    eqMidFreq = apvts.getRawParameterValue("eqMidFreq");
    eqMidGain = apvts.getRawParameterValue("eqMidGain");
    eqMidQ = apvts.getRawParameterValue("eqMidQ");
    eqHighMidFreq = apvts.getRawParameterValue("eqHighMidFreq");
    eqHighMidGain = apvts.getRawParameterValue("eqHighMidGain");
    eqHighMidQ = apvts.getRawParameterValue("eqHighMidQ");
    eqHighShelfFreq = apvts.getRawParameterValue("eqHighShelfFreq");
    eqHighShelfGain = apvts.getRawParameterValue("eqHighShelfGain");
    eqBypass = apvts.getRawParameterValue("eqBypass");

    // Global parameters
    inputGain = apvts.getRawParameterValue("inputGain");
    outputGain = apvts.getRawParameterValue("outputGain");
}

VoxProcAudioProcessor::~VoxProcAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout VoxProcAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === COMPRESSOR ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compThreshold", 1), "Comp Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRatio", 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compAttack", 1), "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRelease", 1), "Comp Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.4f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMakeup", 1), "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compKnee", 1), "Comp Knee",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 6.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compAutoRelease", 1), "Comp Auto Release", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compBypass", 1), "Comp Bypass", false));

    // === DE-ESSER ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("deessFrequency", 1), "De-ess Frequency",
        juce::NormalisableRange<float>(2000.0f, 12000.0f, 10.0f, 0.5f), 6000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("deessThreshold", 1), "De-ess Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("deessRange", 1), "De-ess Range",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 6.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("deessMode", 1), "De-ess Mode",
        juce::StringArray{ "Split-Band", "Wideband" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("deessListen", 1), "De-ess Listen", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("deessBypass", 1), "De-ess Bypass", false));

    // === EQ ===
    // HPF
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHPFFreq", 1), "HPF Frequency",
        juce::NormalisableRange<float>(20.0f, 400.0f, 1.0f, 0.5f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("eqHPFSlope", 1), "HPF Slope",
        juce::StringArray{ "12 dB/oct", "24 dB/oct" }, 0));

    // Low Shelf
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowShelfFreq", 1), "Low Shelf Freq",
        juce::NormalisableRange<float>(50.0f, 500.0f, 1.0f, 0.5f), 200.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowShelfGain", 1), "Low Shelf Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Low-Mid
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowMidFreq", 1), "Low-Mid Freq",
        juce::NormalisableRange<float>(100.0f, 1000.0f, 1.0f, 0.5f), 400.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowMidGain", 1), "Low-Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowMidQ", 1), "Low-Mid Q",
        juce::NormalisableRange<float>(0.5f, 10.0f, 0.01f, 0.5f), 1.0f));

    // Mid
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqMidFreq", 1), "Mid Freq",
        juce::NormalisableRange<float>(500.0f, 4000.0f, 1.0f, 0.5f), 1000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqMidGain", 1), "Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqMidQ", 1), "Mid Q",
        juce::NormalisableRange<float>(0.5f, 10.0f, 0.01f, 0.5f), 1.0f));

    // High-Mid
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighMidFreq", 1), "High-Mid Freq",
        juce::NormalisableRange<float>(2000.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighMidGain", 1), "High-Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighMidQ", 1), "High-Mid Q",
        juce::NormalisableRange<float>(0.5f, 10.0f, 0.01f, 0.5f), 1.0f));

    // High Shelf
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighShelfFreq", 1), "High Shelf Freq",
        juce::NormalisableRange<float>(4000.0f, 16000.0f, 10.0f, 0.5f), 8000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighShelfGain", 1), "High Shelf Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("eqBypass", 1), "EQ Bypass", false));

    // === GLOBAL ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("inputGain", 1), "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return { params.begin(), params.end() };
}

const juce::String VoxProcAudioProcessor::getName() const { return JucePlugin_Name; }
bool VoxProcAudioProcessor::acceptsMidi() const { return false; }
bool VoxProcAudioProcessor::producesMidi() const { return false; }
bool VoxProcAudioProcessor::isMidiEffect() const { return false; }
double VoxProcAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int VoxProcAudioProcessor::getNumPrograms() { return 1; }
int VoxProcAudioProcessor::getCurrentProgram() { return 0; }
void VoxProcAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String VoxProcAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void VoxProcAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

void VoxProcAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    compressor.prepare(sampleRate, samplesPerBlock);
    deEsser.prepare(sampleRate, samplesPerBlock);
    equalizer.prepare(sampleRate, samplesPerBlock);
}

void VoxProcAudioProcessor::releaseResources()
{
    compressor.reset();
    deEsser.reset();
    equalizer.reset();
}

bool VoxProcAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void VoxProcAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Apply input gain
    float inGainLinear = std::pow(10.0f, inputGain->load() / 20.0f);
    buffer.applyGain(inGainLinear);

    // Measure input level (after input gain)
    float inLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        inLevel = std::max(inLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    inputLevel.store(inLevel);

    // Update and process EQ (first in chain - signal flow: HPF -> EQ -> Compressor -> De-Esser)
    equalizer.setHPFFrequency(eqHPFFreq->load());
    equalizer.setHPFSlope(static_cast<int>(eqHPFSlope->load()) == 1 ? 24 : 12);
    equalizer.setLowShelfFrequency(eqLowShelfFreq->load());
    equalizer.setLowShelfGain(eqLowShelfGain->load());
    equalizer.setLowMidFrequency(eqLowMidFreq->load());
    equalizer.setLowMidGain(eqLowMidGain->load());
    equalizer.setLowMidQ(eqLowMidQ->load());
    equalizer.setMidFrequency(eqMidFreq->load());
    equalizer.setMidGain(eqMidGain->load());
    equalizer.setMidQ(eqMidQ->load());
    equalizer.setHighMidFrequency(eqHighMidFreq->load());
    equalizer.setHighMidGain(eqHighMidGain->load());
    equalizer.setHighMidQ(eqHighMidQ->load());
    equalizer.setHighShelfFrequency(eqHighShelfFreq->load());
    equalizer.setHighShelfGain(eqHighShelfGain->load());
    equalizer.setBypass(eqBypass->load() > 0.5f);
    equalizer.process(buffer);

    // Update and process compressor
    compressor.setThreshold(compThreshold->load());
    compressor.setRatio(compRatio->load());
    compressor.setAttack(compAttack->load());
    compressor.setRelease(compRelease->load());
    compressor.setMakeupGain(compMakeup->load());
    compressor.setKnee(compKnee->load());
    compressor.setAutoRelease(compAutoRelease->load() > 0.5f);
    compressor.setBypass(compBypass->load() > 0.5f);
    compressor.process(buffer);

    // Update and process de-esser
    deEsser.setFrequency(deessFrequency->load());
    deEsser.setThreshold(deessThreshold->load());
    deEsser.setRange(deessRange->load());
    deEsser.setMode(static_cast<int>(deessMode->load()));
    deEsser.setListenMode(deessListen->load() > 0.5f);
    deEsser.setBypass(deessBypass->load() > 0.5f);
    deEsser.process(buffer);

    // Apply output gain
    float outGainLinear = std::pow(10.0f, outputGain->load() / 20.0f);
    buffer.applyGain(outGainLinear);

    // Measure output level
    float outLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        outLevel = std::max(outLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    outputLevel.store(outLevel);
}

void VoxProcAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VoxProcAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorEditor* VoxProcAudioProcessor::createEditor()
{
    return new VoxProcAudioProcessorEditor(*this);
}

bool VoxProcAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxProcAudioProcessor();
}
