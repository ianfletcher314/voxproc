#pragma once

#include <JuceHeader.h>
#include "DSP/Compressor.h"
#include "DSP/DeEsser.h"
#include "DSP/Equalizer.h"

class VoxProcAudioProcessor : public juce::AudioProcessor
{
public:
    VoxProcAudioProcessor();
    ~VoxProcAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Metering
    float getCompressorGainReduction() const { return compressor.getGainReduction(); }
    float getDeEsserGainReduction() const { return deEsser.getGainReduction(); }
    bool isDeEsserActive() const { return deEsser.isActive(); }
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }

    // EQ visualization
    float getEQMagnitudeAtFrequency(float freq) const { return equalizer.getMagnitudeAtFrequency(freq); }

    // DSP access for visualization
    const Equalizer& getEqualizer() const { return equalizer; }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP
    Compressor compressor;
    DeEsser deEsser;
    Equalizer equalizer;

    // === COMPRESSOR PARAMETERS ===
    std::atomic<float>* compThreshold = nullptr;
    std::atomic<float>* compRatio = nullptr;
    std::atomic<float>* compAttack = nullptr;
    std::atomic<float>* compRelease = nullptr;
    std::atomic<float>* compMakeup = nullptr;
    std::atomic<float>* compKnee = nullptr;
    std::atomic<float>* compAutoRelease = nullptr;
    std::atomic<float>* compBypass = nullptr;

    // === DE-ESSER PARAMETERS ===
    std::atomic<float>* deessFrequency = nullptr;
    std::atomic<float>* deessThreshold = nullptr;
    std::atomic<float>* deessRange = nullptr;
    std::atomic<float>* deessMode = nullptr;
    std::atomic<float>* deessListen = nullptr;
    std::atomic<float>* deessBypass = nullptr;

    // === EQ PARAMETERS ===
    // HPF
    std::atomic<float>* eqHPFFreq = nullptr;
    std::atomic<float>* eqHPFSlope = nullptr;

    // Low Shelf
    std::atomic<float>* eqLowShelfFreq = nullptr;
    std::atomic<float>* eqLowShelfGain = nullptr;

    // Low-Mid
    std::atomic<float>* eqLowMidFreq = nullptr;
    std::atomic<float>* eqLowMidGain = nullptr;
    std::atomic<float>* eqLowMidQ = nullptr;

    // Mid
    std::atomic<float>* eqMidFreq = nullptr;
    std::atomic<float>* eqMidGain = nullptr;
    std::atomic<float>* eqMidQ = nullptr;

    // High-Mid
    std::atomic<float>* eqHighMidFreq = nullptr;
    std::atomic<float>* eqHighMidGain = nullptr;
    std::atomic<float>* eqHighMidQ = nullptr;

    // High Shelf
    std::atomic<float>* eqHighShelfFreq = nullptr;
    std::atomic<float>* eqHighShelfGain = nullptr;

    std::atomic<float>* eqBypass = nullptr;

    // === GLOBAL PARAMETERS ===
    std::atomic<float>* inputGain = nullptr;
    std::atomic<float>* outputGain = nullptr;

    // Level metering
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxProcAudioProcessor)
};
