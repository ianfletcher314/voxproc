#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Custom LookAndFeel for PDLBRD-style knobs (knurled edge with indicator line)
//==============================================================================
class VoxProcLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VoxProcLookAndFeel() {}

    void setAccentColour(juce::Colour c) { accentColour = c; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float, float,
                          juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(2.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        // Outer ring (knurled edge)
        g.setColour(juce::Colour(0xff303030));
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        // Knurl pattern
        g.setColour(juce::Colour(0xff404040));
        int numKnurls = 24;
        for (int i = 0; i < numKnurls; ++i)
        {
            float angle = i * juce::MathConstants<float>::twoPi / numKnurls;
            float x1 = cx + (radius - 1.0f) * std::cos(angle);
            float y1 = cy + (radius - 1.0f) * std::sin(angle);
            float x2 = cx + (radius - 4.0f) * std::cos(angle);
            float y2 = cy + (radius - 4.0f) * std::sin(angle);
            g.drawLine(x1, y1, x2, y2, 1.5f);
        }

        // Main knob body
        float innerRadius = radius * 0.78f;
        juce::ColourGradient knobGradient(juce::Colour(0xff555555), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                           juce::Colour(0xff252525), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Ring
        g.setColour(juce::Colour(0xff606060));
        g.drawEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

        // Indicator line (7 o'clock to 5 o'clock, clockwise)
        float indicatorAngle = juce::jmap(sliderPosProportional, 0.0f, 1.0f, -1.047f, 4.189f) + juce::MathConstants<float>::pi;
        float indicatorLength = innerRadius * 0.65f;
        float ix1 = cx + (innerRadius * 0.2f) * std::cos(indicatorAngle);
        float iy1 = cy + (innerRadius * 0.2f) * std::sin(indicatorAngle);
        float ix2 = cx + indicatorLength * std::cos(indicatorAngle);
        float iy2 = cy + indicatorLength * std::sin(indicatorAngle);
        g.setColour(accentColour);
        g.drawLine(ix1, iy1, ix2, iy2, 3.0f);

        // Center cap
        float capRadius = innerRadius * 0.25f;
        g.setColour(juce::Colour(0xff404040));
        g.fillEllipse(cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f);
    }

private:
    juce::Colour accentColour = juce::Colours::white;
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
// EQ Visualizer Component - Synth-style with color-coded bands
//==============================================================================
class EQVisualizer : public juce::Component
{
public:
    EQVisualizer(VoxProcAudioProcessor& p) : processor(p) {}
    void paint(juce::Graphics& g) override;

    // Band colors matching synth page style
    static constexpr uint32_t colorHPF       = 0xffff5555;  // Red
    static constexpr uint32_t colorLowShelf  = 0xffffaa00;  // Orange
    static constexpr uint32_t colorLowMid    = 0xff00ff88;  // Green
    static constexpr uint32_t colorMid       = 0xff00aaff;  // Cyan
    static constexpr uint32_t colorHighMid   = 0xffff55ff;  // Magenta
    static constexpr uint32_t colorHighShelf = 0xffffffff;  // White
    static constexpr uint32_t colorCombined  = 0xffffffff;  // White with glow

private:
    VoxProcAudioProcessor& processor;
    float freqToX(float freq, float width) const;
    float dbToY(float db, float height) const;

    // Calculate magnitude for a single band at a given frequency
    float getBandMagnitudeAtFrequency(float freq, int bandIndex) const;
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
