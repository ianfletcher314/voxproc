#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Custom LookAndFeel for VoxProc-style knobs
//==============================================================================
class VoxProcLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VoxProcLookAndFeel() {}

    void setAccentColour(juce::Colour c) { accentColour = c; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float, float,
                          juce::Slider&) override;

private:
    juce::Colour accentColour = juce::Colour(0xff4a9eff);
};

//==============================================================================
// Level Meter Component
//==============================================================================
class LevelMeter : public juce::Component
{
public:
    void setLevel(float newLevel) { level = newLevel; repaint(); }
    void setVertical(bool v) { vertical = v; }
    void paint(juce::Graphics& g) override;

private:
    float level = 0.0f;
    bool vertical = true;
};

//==============================================================================
// Gain Reduction Meter Component
//==============================================================================
class GainReductionMeter : public juce::Component
{
public:
    void setGainReduction(float gr) { gainReduction = gr; repaint(); }
    void paint(juce::Graphics& g) override;

private:
    float gainReduction = 0.0f;
};

//==============================================================================
// EQ Visualizer Component
//==============================================================================
class EQVisualizer : public juce::Component
{
public:
    EQVisualizer(VoxProcAudioProcessor& p) : processor(p) {}
    void paint(juce::Graphics& g) override;

private:
    VoxProcAudioProcessor& processor;
    float freqToX(float freq, float width) const;
    float dbToY(float db, float height) const;
};

//==============================================================================
// Section Components
//==============================================================================
class CompressorSection : public juce::Component
{
public:
    CompressorSection();
    ~CompressorSection() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider, makeupSlider, kneeSlider;
    juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel, makeupLabel, kneeLabel;
    juce::ToggleButton autoReleaseButton { "Auto" };
    juce::ToggleButton bypassButton { "Bypass" };
    GainReductionMeter grMeter;

private:
    std::unique_ptr<VoxProcLookAndFeel> lookAndFeel;
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
};

class DeEsserSection : public juce::Component
{
public:
    DeEsserSection();
    ~DeEsserSection() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider frequencySlider, thresholdSlider, rangeSlider;
    juce::Label frequencyLabel, thresholdLabel, rangeLabel;
    juce::ComboBox modeSelector;
    juce::Label modeLabel;
    juce::ToggleButton listenButton { "Listen" };
    juce::ToggleButton bypassButton { "Bypass" };
    GainReductionMeter grMeter;

    void setActive(bool active) { isActive = active; repaint(); }

private:
    std::unique_ptr<VoxProcLookAndFeel> lookAndFeel;
    bool isActive = false;
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
};

class EQSection : public juce::Component
{
public:
    EQSection(VoxProcAudioProcessor& p);
    ~EQSection() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

    // HPF
    juce::Slider hpfFreqSlider;
    juce::Label hpfFreqLabel;
    juce::ComboBox hpfSlopeSelector;

    // Low Shelf
    juce::Slider lowShelfFreqSlider, lowShelfGainSlider;
    juce::Label lowShelfFreqLabel, lowShelfGainLabel;

    // Low-Mid
    juce::Slider lowMidFreqSlider, lowMidGainSlider, lowMidQSlider;
    juce::Label lowMidFreqLabel, lowMidGainLabel, lowMidQLabel;

    // Mid
    juce::Slider midFreqSlider, midGainSlider, midQSlider;
    juce::Label midFreqLabel, midGainLabel, midQLabel;

    // High-Mid
    juce::Slider highMidFreqSlider, highMidGainSlider, highMidQSlider;
    juce::Label highMidFreqLabel, highMidGainLabel, highMidQLabel;

    // High Shelf
    juce::Slider highShelfFreqSlider, highShelfGainSlider;
    juce::Label highShelfFreqLabel, highShelfGainLabel;

    juce::ToggleButton bypassButton { "Bypass" };

    EQVisualizer eqVisualizer;

private:
    std::unique_ptr<VoxProcLookAndFeel> lookAndFeel;
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void setupGainSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
};

//==============================================================================
// Main Editor
//==============================================================================
class VoxProcAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    VoxProcAudioProcessorEditor(VoxProcAudioProcessor&);
    ~VoxProcAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    VoxProcAudioProcessor& audioProcessor;

    // Sections
    CompressorSection compressorSection;
    DeEsserSection deEsserSection;
    EQSection eqSection;

    // Global controls
    juce::Slider inputGainSlider, outputGainSlider;
    juce::Label inputGainLabel, outputGainLabel;
    LevelMeter inputMeter, outputMeter;

    float smoothedInputLevel = 0.0f;
    float smoothedOutputLevel = 0.0f;

    std::unique_ptr<VoxProcLookAndFeel> globalLookAndFeel;

    // Attachments
    // Compressor
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compKneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compAutoReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compBypassAttachment;

    // De-esser
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessFrequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessRangeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> deessModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deessListenAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deessBypassAttachment;

    // EQ
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHPFFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> eqHPFSlopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowShelfFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowShelfGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowMidFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowMidGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowMidQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqMidFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqMidGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqMidQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighMidFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighMidGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighMidQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighShelfFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighShelfGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqBypassAttachment;

    // Global
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxProcAudioProcessorEditor)
};
