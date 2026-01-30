#pragma once

#include <JuceHeader.h>
#include "DSP/Compressor.h"
#include "DSP/DeEsser.h"
#include "DSP/Equalizer.h"

// FFT size for spectrum analyzer
static constexpr int fftOrder = 11;  // 2^11 = 2048 samples
static constexpr int fftSize = 1 << fftOrder;

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
    float getEQBandMagnitudeAtFrequency(float freq, int bandIndex) const { return equalizer.getBandMagnitudeAtFrequency(freq, bandIndex); }

    // DSP access for visualization
    const Equalizer& getEqualizer() const { return equalizer; }

    // Update EQ parameters for visualization (call from editor timer)
    void updateEQForVisualization()
    {
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
    }

    // Spectrum analyzer data
    const std::array<float, fftSize / 2>& getInputSpectrum() const { return inputSpectrum; }
    const std::array<float, fftSize / 2>& getOutputSpectrum() const { return outputSpectrum; }
    double getCurrentSampleRate() const { return currentSampleRate; }

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

    // FFT for spectrum analyzer
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, fftSize * 2> inputFFTData {};
    std::array<float, fftSize * 2> outputFFTData {};
    std::array<float, fftSize / 2> inputSpectrum {};
    std::array<float, fftSize / 2> outputSpectrum {};

    std::array<float, fftSize> inputFifo {};
    std::array<float, fftSize> outputFifo {};
    int fifoIndex = 0;
    bool fftDataReady = false;

    double currentSampleRate = 44100.0;

    void pushSamplesToFFT(const float* inputData, const float* outputData, int numSamples);
    void processFFT();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxProcAudioProcessor)
};
