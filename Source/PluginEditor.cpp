#include "PluginProcessor.h"
#include "PluginEditor.h"

// Color scheme
namespace VoxColors {
    const juce::Colour background = juce::Colour(0xff1a1a2e);
    const juce::Colour panelBg = juce::Colour(0xff16213e);
    const juce::Colour panelBorder = juce::Colour(0xff0f3460);
    const juce::Colour accent = juce::Colour(0xff4a9eff);
    const juce::Colour accentOrange = juce::Colour(0xffe94560);
    const juce::Colour accentGreen = juce::Colour(0xff00d4aa);
    const juce::Colour textPrimary = juce::Colour(0xffeaeaea);
    const juce::Colour textSecondary = juce::Colour(0xff888888);
    const juce::Colour meterGreen = juce::Colour(0xff22c55e);
    const juce::Colour meterYellow = juce::Colour(0xffeab308);
    const juce::Colour meterRed = juce::Colour(0xffef4444);
}

//==============================================================================
// VoxProcLookAndFeel
//==============================================================================
void VoxProcLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPosProportional, float, float,
                                           juce::Slider&)
{
    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(2.0f);
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

    // Outer ring
    g.setColour(juce::Colour(0xff2a2a4a));
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    // Main knob body
    float innerRadius = radius * 0.8f;
    juce::ColourGradient knobGradient(juce::Colour(0xff3a3a5a), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                       juce::Colour(0xff252535), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
    g.setGradientFill(knobGradient);
    g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

    // Indicator arc (background)
    juce::Path arcBg;
    float arcRadius = radius - 3.0f;
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;
    arcBg.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xff1a1a2a));
    g.strokePath(arcBg, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Indicator arc (value)
    juce::Path arcVal;
    float valueAngle = startAngle + sliderPosProportional * (endAngle - startAngle);
    arcVal.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, valueAngle, true);
    g.setColour(accentColour);
    g.strokePath(arcVal, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Indicator dot
    float indicatorAngle = startAngle + sliderPosProportional * (endAngle - startAngle);
    float dotRadius = 4.0f;
    float dotDistance = innerRadius * 0.65f;
    float dotX = cx + dotDistance * std::cos(indicatorAngle);
    float dotY = cy + dotDistance * std::sin(indicatorAngle);
    g.setColour(accentColour);
    g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
}

//==============================================================================
// LevelMeter
//==============================================================================
void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xff151520));
    g.fillRoundedRectangle(bounds, 3.0f);

    // Calculate level
    float db = juce::Decibels::gainToDecibels(level, -60.0f);
    float normalized = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);
    normalized = juce::jlimit(0.0f, 1.0f, normalized);

    // Gradient color
    juce::Colour barColour;
    if (normalized < 0.6f)
        barColour = VoxColors::meterGreen;
    else if (normalized < 0.85f)
        barColour = VoxColors::meterYellow;
    else
        barColour = VoxColors::meterRed;

    if (vertical)
    {
        float barHeight = bounds.getHeight() * normalized;
        auto barBounds = bounds.removeFromBottom(barHeight);
        g.setColour(barColour);
        g.fillRoundedRectangle(barBounds.reduced(1), 2.0f);
    }
    else
    {
        float barWidth = bounds.getWidth() * normalized;
        auto barBounds = bounds.removeFromLeft(barWidth);
        g.setColour(barColour);
        g.fillRoundedRectangle(barBounds.reduced(1), 2.0f);
    }

    // Outline
    g.setColour(juce::Colour(0xff333344));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 3.0f, 1.0f);
}

//==============================================================================
// GainReductionMeter
//==============================================================================
void GainReductionMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Segmented LED meter
    const int numSegments = 12;
    float segmentWidth = (bounds.getWidth() - 10) / numSegments;
    float segmentHeight = bounds.getHeight() - 8;
    float segmentGap = 2.0f;

    // Gain reduction is in dB (0 to ~18dB for full scale)
    float grDb = juce::jlimit(0.0f, 18.0f, gainReduction);
    int litSegments = (int)((grDb / 18.0f) * numSegments);

    for (int i = 0; i < numSegments; ++i)
    {
        float segX = bounds.getX() + 5 + i * segmentWidth;

        juce::Colour segColour;
        if (i < 6)
            segColour = VoxColors::meterGreen;
        else if (i < 9)
            segColour = VoxColors::meterYellow;
        else
            segColour = VoxColors::meterRed;

        bool isLit = i < litSegments;
        g.setColour(isLit ? segColour : segColour.withAlpha(0.15f));
        g.fillRoundedRectangle(segX, bounds.getY() + 4, segmentWidth - segmentGap, segmentHeight, 2.0f);
    }

    // Border
    g.setColour(juce::Colour(0xff333344));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

//==============================================================================
// EQVisualizer
//==============================================================================
float EQVisualizer::freqToX(float freq, float width) const
{
    // Log scale: 20Hz to 20kHz
    float minLog = std::log10(20.0f);
    float maxLog = std::log10(20000.0f);
    float freqLog = std::log10(freq);
    return width * (freqLog - minLog) / (maxLog - minLog);
}

float EQVisualizer::dbToY(float db, float height) const
{
    // -18dB to +18dB range
    return height * 0.5f * (1.0f - db / 18.0f);
}

void EQVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xff0a0a15));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Grid lines
    g.setColour(juce::Colour(0xff1a1a2a));

    // Horizontal grid (dB)
    for (int db = -12; db <= 12; db += 6)
    {
        float y = dbToY((float)db, bounds.getHeight());
        g.drawHorizontalLine((int)(bounds.getY() + y), bounds.getX() + 5, bounds.getRight() - 5);
    }

    // Zero line
    g.setColour(juce::Colour(0xff2a2a4a));
    float zeroY = dbToY(0.0f, bounds.getHeight());
    g.drawHorizontalLine((int)(bounds.getY() + zeroY), bounds.getX() + 5, bounds.getRight() - 5);

    // Vertical grid (frequency)
    float freqs[] = { 100.0f, 1000.0f, 10000.0f };
    g.setColour(juce::Colour(0xff1a1a2a));
    for (float freq : freqs)
    {
        float x = freqToX(freq, bounds.getWidth());
        g.drawVerticalLine((int)(bounds.getX() + x), bounds.getY() + 5, bounds.getBottom() - 5);
    }

    // Draw frequency response curve
    juce::Path responsePath;
    bool pathStarted = false;

    for (float x = 0; x < bounds.getWidth(); x += 2.0f)
    {
        float freq = 20.0f * std::pow(1000.0f, x / bounds.getWidth());
        float magnitude = processor.getEQMagnitudeAtFrequency(freq);
        float db = juce::Decibels::gainToDecibels(magnitude, -24.0f);
        db = juce::jlimit(-18.0f, 18.0f, db);

        float y = dbToY(db, bounds.getHeight());

        if (!pathStarted)
        {
            responsePath.startNewSubPath(bounds.getX() + x, bounds.getY() + y);
            pathStarted = true;
        }
        else
        {
            responsePath.lineTo(bounds.getX() + x, bounds.getY() + y);
        }
    }

    // Fill under curve
    juce::Path fillPath = responsePath;
    fillPath.lineTo(bounds.getRight(), bounds.getY() + zeroY);
    fillPath.lineTo(bounds.getX(), bounds.getY() + zeroY);
    fillPath.closeSubPath();

    g.setColour(VoxColors::accent.withAlpha(0.15f));
    g.fillPath(fillPath);

    // Stroke curve
    g.setColour(VoxColors::accent);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));

    // Border
    g.setColour(VoxColors::panelBorder);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
}

//==============================================================================
// CompressorSection
//==============================================================================
CompressorSection::CompressorSection()
{
    lookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    lookAndFeel->setAccentColour(VoxColors::accentOrange);

    setupSlider(thresholdSlider, thresholdLabel, "THRESH");
    setupSlider(ratioSlider, ratioLabel, "RATIO");
    setupSlider(attackSlider, attackLabel, "ATTACK");
    setupSlider(releaseSlider, releaseLabel, "RELEASE");
    setupSlider(makeupSlider, makeupLabel, "MAKEUP");
    setupSlider(kneeSlider, kneeLabel, "KNEE");

    thresholdSlider.setTextValueSuffix(" dB");
    ratioSlider.setTextValueSuffix(":1");
    attackSlider.setTextValueSuffix(" ms");
    releaseSlider.setTextValueSuffix(" ms");
    makeupSlider.setTextValueSuffix(" dB");
    kneeSlider.setTextValueSuffix(" dB");

    autoReleaseButton.setColour(juce::ToggleButton::textColourId, VoxColors::textSecondary);
    autoReleaseButton.setColour(juce::ToggleButton::tickColourId, VoxColors::accentOrange);
    addAndMakeVisible(autoReleaseButton);

    bypassButton.setColour(juce::ToggleButton::textColourId, VoxColors::textSecondary);
    bypassButton.setColour(juce::ToggleButton::tickColourId, VoxColors::accentOrange);
    addAndMakeVisible(bypassButton);

    addAndMakeVisible(grMeter);
}

CompressorSection::~CompressorSection()
{
    thresholdSlider.setLookAndFeel(nullptr);
    ratioSlider.setLookAndFeel(nullptr);
    attackSlider.setLookAndFeel(nullptr);
    releaseSlider.setLookAndFeel(nullptr);
    makeupSlider.setLookAndFeel(nullptr);
    kneeSlider.setLookAndFeel(nullptr);
}

void CompressorSection::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 14);
    slider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void CompressorSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2);

    // Panel background
    g.setColour(VoxColors::panelBg);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(VoxColors::panelBorder);
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    // Title
    g.setColour(VoxColors::accentOrange);
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("COMPRESSOR", bounds.removeFromTop(30).reduced(10, 0), juce::Justification::centredLeft);
}

void CompressorSection::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(30); // Title

    const int knobSize = 55;
    const int labelHeight = 14;
    const int spacing = 62;

    int x = 10;
    int y = bounds.getY();

    auto placeKnob = [&](juce::Slider& slider, juce::Label& label)
    {
        label.setBounds(x, y, knobSize, labelHeight);
        slider.setBounds(x, y + labelHeight, knobSize, knobSize);
        x += spacing;
    };

    placeKnob(thresholdSlider, thresholdLabel);
    placeKnob(ratioSlider, ratioLabel);
    placeKnob(attackSlider, attackLabel);
    placeKnob(releaseSlider, releaseLabel);
    placeKnob(makeupSlider, makeupLabel);
    placeKnob(kneeSlider, kneeLabel);

    // GR Meter
    grMeter.setBounds(x + 10, y + 20, getWidth() - x - 80, 24);

    // Buttons
    autoReleaseButton.setBounds(x + 10, y + 50, 60, 20);
    bypassButton.setBounds(getWidth() - 70, y + 50, 60, 20);
}

//==============================================================================
// DeEsserSection
//==============================================================================
DeEsserSection::DeEsserSection()
{
    lookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    lookAndFeel->setAccentColour(VoxColors::accentGreen);

    setupSlider(frequencySlider, frequencyLabel, "FREQUENCY");
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(rangeSlider, rangeLabel, "RANGE");

    frequencySlider.setTextValueSuffix(" Hz");
    thresholdSlider.setTextValueSuffix(" dB");
    rangeSlider.setTextValueSuffix(" dB");

    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    modeLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(modeLabel);

    modeSelector.addItem("Split-Band", 1);
    modeSelector.addItem("Wideband", 2);
    modeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1a2a));
    modeSelector.setColour(juce::ComboBox::textColourId, VoxColors::accentGreen);
    modeSelector.setColour(juce::ComboBox::outlineColourId, VoxColors::panelBorder);
    addAndMakeVisible(modeSelector);

    listenButton.setColour(juce::ToggleButton::textColourId, VoxColors::textSecondary);
    listenButton.setColour(juce::ToggleButton::tickColourId, VoxColors::accentGreen);
    addAndMakeVisible(listenButton);

    bypassButton.setColour(juce::ToggleButton::textColourId, VoxColors::textSecondary);
    bypassButton.setColour(juce::ToggleButton::tickColourId, VoxColors::accentGreen);
    addAndMakeVisible(bypassButton);

    addAndMakeVisible(grMeter);
}

DeEsserSection::~DeEsserSection()
{
    frequencySlider.setLookAndFeel(nullptr);
    thresholdSlider.setLookAndFeel(nullptr);
    rangeSlider.setLookAndFeel(nullptr);
}

void DeEsserSection::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 14);
    slider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void DeEsserSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2);

    g.setColour(VoxColors::panelBg);
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(VoxColors::panelBorder);
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    // Title with activity indicator
    auto titleArea = bounds.removeFromTop(30).reduced(10, 0);
    g.setColour(VoxColors::accentGreen);
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("DE-ESSER", titleArea, juce::Justification::centredLeft);

    // Activity LED
    float ledX = titleArea.getRight() - 12;
    float ledY = titleArea.getCentreY() - 5;
    if (isActive)
    {
        g.setColour(VoxColors::accentGreen.withAlpha(0.4f));
        g.fillEllipse(ledX - 3, ledY - 3, 16, 16);
        g.setColour(VoxColors::accentGreen);
    }
    else
    {
        g.setColour(VoxColors::accentGreen.withAlpha(0.2f));
    }
    g.fillEllipse(ledX, ledY, 10, 10);
}

void DeEsserSection::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(30);

    const int knobSize = 55;
    const int labelHeight = 14;
    const int spacing = 62;

    int x = 10;
    int y = bounds.getY();

    auto placeKnob = [&](juce::Slider& slider, juce::Label& label)
    {
        label.setBounds(x, y, knobSize, labelHeight);
        slider.setBounds(x, y + labelHeight, knobSize, knobSize);
        x += spacing;
    };

    placeKnob(frequencySlider, frequencyLabel);
    placeKnob(thresholdSlider, thresholdLabel);
    placeKnob(rangeSlider, rangeLabel);

    // Mode selector
    modeLabel.setBounds(x, y, 80, labelHeight);
    modeSelector.setBounds(x, y + labelHeight + 5, 85, 24);

    // GR Meter
    grMeter.setBounds(x + 100, y + 10, getWidth() - x - 180, 24);

    // Buttons
    listenButton.setBounds(x + 100, y + 45, 60, 20);
    bypassButton.setBounds(getWidth() - 70, y + 45, 60, 20);
}

//==============================================================================
// EQSection
//==============================================================================
EQSection::EQSection(VoxProcAudioProcessor& p)
    : eqVisualizer(p)
{
    lookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    lookAndFeel->setAccentColour(VoxColors::accent);

    // HPF
    setupSlider(hpfFreqSlider, hpfFreqLabel, "HPF");
    hpfFreqSlider.setTextValueSuffix(" Hz");

    hpfSlopeSelector.addItem("12 dB/oct", 1);
    hpfSlopeSelector.addItem("24 dB/oct", 2);
    hpfSlopeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1a2a));
    hpfSlopeSelector.setColour(juce::ComboBox::textColourId, VoxColors::accent);
    hpfSlopeSelector.setColour(juce::ComboBox::outlineColourId, VoxColors::panelBorder);
    addAndMakeVisible(hpfSlopeSelector);

    // Low Shelf
    setupGainSlider(lowShelfGainSlider, lowShelfGainLabel, "LOW SHELF");
    setupSlider(lowShelfFreqSlider, lowShelfFreqLabel, "FREQ");
    lowShelfFreqSlider.setTextValueSuffix(" Hz");
    lowShelfGainSlider.setTextValueSuffix(" dB");

    // Low-Mid
    setupGainSlider(lowMidGainSlider, lowMidGainLabel, "LOW-MID");
    setupSlider(lowMidFreqSlider, lowMidFreqLabel, "FREQ");
    setupSlider(lowMidQSlider, lowMidQLabel, "Q");
    lowMidFreqSlider.setTextValueSuffix(" Hz");
    lowMidGainSlider.setTextValueSuffix(" dB");

    // Mid
    setupGainSlider(midGainSlider, midGainLabel, "MID");
    setupSlider(midFreqSlider, midFreqLabel, "FREQ");
    setupSlider(midQSlider, midQLabel, "Q");
    midFreqSlider.setTextValueSuffix(" Hz");
    midGainSlider.setTextValueSuffix(" dB");

    // High-Mid
    setupGainSlider(highMidGainSlider, highMidGainLabel, "HIGH-MID");
    setupSlider(highMidFreqSlider, highMidFreqLabel, "FREQ");
    setupSlider(highMidQSlider, highMidQLabel, "Q");
    highMidFreqSlider.setTextValueSuffix(" Hz");
    highMidGainSlider.setTextValueSuffix(" dB");

    // High Shelf
    setupGainSlider(highShelfGainSlider, highShelfGainLabel, "HIGH SHELF");
    setupSlider(highShelfFreqSlider, highShelfFreqLabel, "FREQ");
    highShelfFreqSlider.setTextValueSuffix(" Hz");
    highShelfGainSlider.setTextValueSuffix(" dB");

    bypassButton.setColour(juce::ToggleButton::textColourId, VoxColors::textSecondary);
    bypassButton.setColour(juce::ToggleButton::tickColourId, VoxColors::accent);
    addAndMakeVisible(bypassButton);

    addAndMakeVisible(eqVisualizer);
}

EQSection::~EQSection()
{
    hpfFreqSlider.setLookAndFeel(nullptr);
    lowShelfFreqSlider.setLookAndFeel(nullptr);
    lowShelfGainSlider.setLookAndFeel(nullptr);
    lowMidFreqSlider.setLookAndFeel(nullptr);
    lowMidGainSlider.setLookAndFeel(nullptr);
    lowMidQSlider.setLookAndFeel(nullptr);
    midFreqSlider.setLookAndFeel(nullptr);
    midGainSlider.setLookAndFeel(nullptr);
    midQSlider.setLookAndFeel(nullptr);
    highMidFreqSlider.setLookAndFeel(nullptr);
    highMidGainSlider.setLookAndFeel(nullptr);
    highMidQSlider.setLookAndFeel(nullptr);
    highShelfFreqSlider.setLookAndFeel(nullptr);
    highShelfGainSlider.setLookAndFeel(nullptr);
}

void EQSection::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 12);
    slider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    label.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void EQSection::setupGainSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 12);
    slider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, VoxColors::accent);
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void EQSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2);

    g.setColour(VoxColors::panelBg);
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(VoxColors::panelBorder);
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    g.setColour(VoxColors::accent);
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("EQUALIZER", bounds.removeFromTop(30).reduced(10, 0), juce::Justification::centredLeft);
}

void EQSection::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(30);

    // EQ Visualizer at top
    eqVisualizer.setBounds(bounds.removeFromTop(100).reduced(0, 5));

    bounds.removeFromTop(10);

    const int knobSize = 45;
    const int smallKnobSize = 38;
    const int labelHeight = 12;
    const int bandWidth = 95;

    int x = 5;
    int y = bounds.getY();

    // HPF
    hpfFreqLabel.setBounds(x, y, knobSize + 10, labelHeight);
    hpfFreqSlider.setBounds(x, y + labelHeight, knobSize + 10, knobSize);
    hpfSlopeSelector.setBounds(x, y + labelHeight + knobSize + 2, 55, 18);
    x += bandWidth - 15;

    // Low Shelf
    lowShelfGainLabel.setBounds(x, y, knobSize, labelHeight);
    lowShelfGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    lowShelfFreqLabel.setBounds(x + knobSize - 5, y + 20, smallKnobSize, 10);
    lowShelfFreqSlider.setBounds(x + knobSize - 5, y + 28, smallKnobSize, smallKnobSize);
    x += bandWidth;

    // Low-Mid
    lowMidGainLabel.setBounds(x, y, knobSize, labelHeight);
    lowMidGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    lowMidFreqLabel.setBounds(x + knobSize - 5, y + 8, smallKnobSize, 10);
    lowMidFreqSlider.setBounds(x + knobSize - 5, y + 16, smallKnobSize, smallKnobSize);
    lowMidQLabel.setBounds(x + knobSize - 5, y + 50, smallKnobSize, 10);
    lowMidQSlider.setBounds(x + knobSize - 5, y + 58, smallKnobSize, smallKnobSize);
    x += bandWidth;

    // Mid
    midGainLabel.setBounds(x, y, knobSize, labelHeight);
    midGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    midFreqLabel.setBounds(x + knobSize - 5, y + 8, smallKnobSize, 10);
    midFreqSlider.setBounds(x + knobSize - 5, y + 16, smallKnobSize, smallKnobSize);
    midQLabel.setBounds(x + knobSize - 5, y + 50, smallKnobSize, 10);
    midQSlider.setBounds(x + knobSize - 5, y + 58, smallKnobSize, smallKnobSize);
    x += bandWidth;

    // High-Mid
    highMidGainLabel.setBounds(x, y, knobSize, labelHeight);
    highMidGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    highMidFreqLabel.setBounds(x + knobSize - 5, y + 8, smallKnobSize, 10);
    highMidFreqSlider.setBounds(x + knobSize - 5, y + 16, smallKnobSize, smallKnobSize);
    highMidQLabel.setBounds(x + knobSize - 5, y + 50, smallKnobSize, 10);
    highMidQSlider.setBounds(x + knobSize - 5, y + 58, smallKnobSize, smallKnobSize);
    x += bandWidth;

    // High Shelf
    highShelfGainLabel.setBounds(x, y, knobSize, labelHeight);
    highShelfGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    highShelfFreqLabel.setBounds(x + knobSize - 5, y + 20, smallKnobSize, 10);
    highShelfFreqSlider.setBounds(x + knobSize - 5, y + 28, smallKnobSize, smallKnobSize);

    bypassButton.setBounds(getWidth() - 70, y + 80, 60, 20);
}

//==============================================================================
// VoxProcAudioProcessorEditor
//==============================================================================
VoxProcAudioProcessorEditor::VoxProcAudioProcessorEditor(VoxProcAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), eqSection(p)
{
    globalLookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    globalLookAndFeel->setAccentColour(VoxColors::textPrimary);

    // Input/Output gain sliders
    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    inputGainSlider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    inputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    inputGainSlider.setTextValueSuffix(" dB");
    inputGainSlider.setLookAndFeel(globalLookAndFeel.get());
    addAndMakeVisible(inputGainSlider);

    inputGainLabel.setText("INPUT", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    inputGainLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(inputGainLabel);

    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    outputGainSlider.setTextValueSuffix(" dB");
    outputGainSlider.setLookAndFeel(globalLookAndFeel.get());
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    outputGainLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(outputGainLabel);

    inputMeter.setVertical(true);
    outputMeter.setVertical(true);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    addAndMakeVisible(compressorSection);
    addAndMakeVisible(deEsserSection);
    addAndMakeVisible(eqSection);

    // Create attachments
    auto& apvts = audioProcessor.getAPVTS();

    // Compressor
    compThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compThreshold", compressorSection.thresholdSlider);
    compRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compRatio", compressorSection.ratioSlider);
    compAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compAttack", compressorSection.attackSlider);
    compReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compRelease", compressorSection.releaseSlider);
    compMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compMakeup", compressorSection.makeupSlider);
    compKneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compKnee", compressorSection.kneeSlider);
    compAutoReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "compAutoRelease", compressorSection.autoReleaseButton);
    compBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "compBypass", compressorSection.bypassButton);

    // De-esser
    deessFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "deessFrequency", deEsserSection.frequencySlider);
    deessThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "deessThreshold", deEsserSection.thresholdSlider);
    deessRangeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "deessRange", deEsserSection.rangeSlider);
    deessModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "deessMode", deEsserSection.modeSelector);
    deessListenAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "deessListen", deEsserSection.listenButton);
    deessBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "deessBypass", deEsserSection.bypassButton);

    // EQ
    eqHPFFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqHPFFreq", eqSection.hpfFreqSlider);
    eqHPFSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "eqHPFSlope", eqSection.hpfSlopeSelector);
    eqLowShelfFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqLowShelfFreq", eqSection.lowShelfFreqSlider);
    eqLowShelfGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqLowShelfGain", eqSection.lowShelfGainSlider);
    eqLowMidFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqLowMidFreq", eqSection.lowMidFreqSlider);
    eqLowMidGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqLowMidGain", eqSection.lowMidGainSlider);
    eqLowMidQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqLowMidQ", eqSection.lowMidQSlider);
    eqMidFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqMidFreq", eqSection.midFreqSlider);
    eqMidGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqMidGain", eqSection.midGainSlider);
    eqMidQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqMidQ", eqSection.midQSlider);
    eqHighMidFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqHighMidFreq", eqSection.highMidFreqSlider);
    eqHighMidGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqHighMidGain", eqSection.highMidGainSlider);
    eqHighMidQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqHighMidQ", eqSection.highMidQSlider);
    eqHighShelfFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqHighShelfFreq", eqSection.highShelfFreqSlider);
    eqHighShelfGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "eqHighShelfGain", eqSection.highShelfGainSlider);
    eqBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "eqBypass", eqSection.bypassButton);

    // Global
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "inputGain", inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "outputGain", outputGainSlider);

    setSize(700, 600);
    startTimerHz(30);
}

VoxProcAudioProcessorEditor::~VoxProcAudioProcessorEditor()
{
    stopTimer();
    inputGainSlider.setLookAndFeel(nullptr);
    outputGainSlider.setLookAndFeel(nullptr);
}

void VoxProcAudioProcessorEditor::timerCallback()
{
    // Update levels
    float targetIn = audioProcessor.getInputLevel();
    float targetOut = audioProcessor.getOutputLevel();

    smoothedInputLevel = smoothedInputLevel * 0.8f + targetIn * 0.2f;
    smoothedOutputLevel = smoothedOutputLevel * 0.8f + targetOut * 0.2f;

    if (targetIn < smoothedInputLevel) smoothedInputLevel *= 0.92f;
    if (targetOut < smoothedOutputLevel) smoothedOutputLevel *= 0.92f;

    inputMeter.setLevel(smoothedInputLevel);
    outputMeter.setLevel(smoothedOutputLevel);

    // Update gain reduction meters
    compressorSection.grMeter.setGainReduction(audioProcessor.getCompressorGainReduction());
    deEsserSection.grMeter.setGainReduction(audioProcessor.getDeEsserGainReduction());
    deEsserSection.setActive(audioProcessor.isDeEsserActive());

    // Repaint EQ visualizer
    eqSection.eqVisualizer.repaint();
}

void VoxProcAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(VoxColors::background);

    // Header
    juce::ColourGradient headerGradient(juce::Colour(0xff252545), 0.0f, 0.0f,
                                         VoxColors::background, 0.0f, 50.0f, false);
    g.setGradientFill(headerGradient);
    g.fillRect(0, 0, getWidth(), 50);

    // Header line
    g.setColour(VoxColors::panelBorder);
    g.fillRect(0, 48, getWidth(), 2);

    // Title
    g.setColour(VoxColors::accent);
    g.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    g.drawText("VOXPROC", 15, 10, 200, 30, juce::Justification::centredLeft);

    g.setColour(VoxColors::textSecondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Vocal Processing Plugin", 15, 32, 200, 15, juce::Justification::centredLeft);
}

void VoxProcAudioProcessorEditor::resized()
{
    const int headerHeight = 50;
    const int margin = 10;
    const int meterWidth = 16;
    const int gainKnobSize = 50;

    auto bounds = getLocalBounds();
    bounds.removeFromTop(headerHeight);

    // Left side: Input meter and gain
    auto leftArea = bounds.removeFromLeft(70);
    inputGainLabel.setBounds(leftArea.getX() + 10, leftArea.getY() + 5, 50, 14);
    inputGainSlider.setBounds(leftArea.getX() + 10, leftArea.getY() + 17, gainKnobSize, gainKnobSize);
    inputMeter.setBounds(leftArea.getX() + 10, leftArea.getY() + 75, meterWidth, leftArea.getHeight() - 85);

    // Right side: Output meter and gain
    auto rightArea = bounds.removeFromRight(70);
    outputGainLabel.setBounds(rightArea.getX() + 10, rightArea.getY() + 5, 50, 14);
    outputGainSlider.setBounds(rightArea.getX() + 10, rightArea.getY() + 17, gainKnobSize, gainKnobSize);
    outputMeter.setBounds(rightArea.getX() + 10, rightArea.getY() + 75, meterWidth, rightArea.getHeight() - 85);

    // Main content area
    bounds.reduce(margin, margin);

    // Compressor section
    compressorSection.setBounds(bounds.removeFromTop(110));
    bounds.removeFromTop(margin);

    // De-esser section
    deEsserSection.setBounds(bounds.removeFromTop(110));
    bounds.removeFromTop(margin);

    // EQ section (takes remaining space)
    eqSection.setBounds(bounds);
}
