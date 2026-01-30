#include "PluginProcessor.h"
#include "PluginEditor.h"

// PDLBRD-style earth tone colors (muted metallic look)
namespace VoxColors {
    const juce::Colour background   = juce::Colour(0xff1a1a1a);  // Dark background
    const juce::Colour panelComp    = juce::Colour(0xff8b4513);  // Saddle brown (compressor)
    const juce::Colour panelDeEss   = juce::Colour(0xff2c5545);  // Dark teal (de-esser)
    const juce::Colour panelEQ      = juce::Colour(0xff4a5568);  // Muted gray (EQ)
    const juce::Colour panelBorder  = juce::Colour(0xff333333);
    const juce::Colour textPrimary  = juce::Colour(0xffeaeaea);
    const juce::Colour textSecondary = juce::Colour(0xffaaaaaa);
    const juce::Colour metal        = juce::Colour(0xffc0c0c0);  // Brushed metal
    const juce::Colour screw        = juce::Colour(0xff505050);
    const juce::Colour led          = juce::Colour(0xffff3333);  // LED red
    const juce::Colour meterGreen   = juce::Colour(0xff22c55e);
    const juce::Colour meterYellow  = juce::Colour(0xffeab308);
    const juce::Colour meterRed     = juce::Colour(0xffef4444);
    const juce::Colour lcdGreen     = juce::Colour(0xff88cc88);  // LCD display green
}

//==============================================================================
// Helper functions for drawing PDLBRD-style components
//==============================================================================
static void drawScrew(juce::Graphics& g, float x, float y, float size)
{
    // Outer ring
    g.setColour(VoxColors::screw.darker(0.3f));
    g.fillEllipse(x - size/2, y - size/2, size, size);

    // Inner hex pattern
    g.setColour(VoxColors::screw.brighter(0.2f));
    g.fillEllipse(x - size/3, y - size/3, size * 0.66f, size * 0.66f);

    // Hex slot
    juce::Path hex;
    float r = size * 0.22f;
    for (int i = 0; i < 6; ++i)
    {
        float angle = i * juce::MathConstants<float>::pi / 3.0f - juce::MathConstants<float>::pi / 6.0f;
        float px = x + r * std::cos(angle);
        float py = y + r * std::sin(angle);
        if (i == 0)
            hex.startNewSubPath(px, py);
        else
            hex.lineTo(px, py);
    }
    hex.closeSubPath();
    g.setColour(VoxColors::screw.darker(0.5f));
    g.fillPath(hex);
}

static void drawBrushedMetalTexture(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour baseColour)
{
    // Base color
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 10.0f);

    // Subtle diagonal brush strokes
    g.setColour(juce::Colours::white.withAlpha(0.03f));
    for (float i = -bounds.getHeight(); i < bounds.getWidth() + bounds.getHeight(); i += 3.0f)
    {
        g.drawLine(bounds.getX() + i, bounds.getY(),
                   bounds.getX() + i + bounds.getHeight(), bounds.getBottom(), 0.5f);
    }

    // Top highlight
    juce::ColourGradient topHighlight(baseColour.brighter(0.15f), bounds.getX(), bounds.getY(),
                                       baseColour, bounds.getX(), bounds.getY() + 25.0f, false);
    g.setGradientFill(topHighlight);
    g.fillRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), 25.0f, 10.0f);
}

static void drawFootswitch(juce::Graphics& g, juce::Rectangle<float> bounds, bool isOn, juce::Colour ledColour)
{
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();

    // Metal housing/bezel
    auto housingBounds = bounds.expanded(2.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(housingBounds, 4.0f);
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRoundedRectangle(housingBounds, 4.0f, 1.0f);

    // Switch base (recessed area)
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRoundedRectangle(bounds, 3.0f);

    // The actual switch button - 3D raised look
    auto buttonBounds = bounds.reduced(4.0f);
    float pressOffset = isOn ? 1.0f : 0.0f;
    buttonBounds = buttonBounds.translated(0, pressOffset);

    // Button shadow (when raised/off)
    if (!isOn)
    {
        g.setColour(juce::Colour(0xff000000));
        g.fillRoundedRectangle(buttonBounds.translated(0, 2), 3.0f);
    }

    // Button top surface gradient
    juce::ColourGradient buttonGrad(
        isOn ? juce::Colour(0xff353535) : juce::Colour(0xff484848),
        buttonBounds.getX(), buttonBounds.getY(),
        isOn ? juce::Colour(0xff252525) : juce::Colour(0xff383838),
        buttonBounds.getX(), buttonBounds.getBottom(), false);
    g.setGradientFill(buttonGrad);
    g.fillRoundedRectangle(buttonBounds, 3.0f);

    // Button edge highlight
    g.setColour(isOn ? juce::Colour(0xff404040) : juce::Colour(0xff555555));
    g.drawRoundedRectangle(buttonBounds, 3.0f, 1.0f);

    // Center ridge/bump on button
    float ridgeWidth = buttonBounds.getWidth() * 0.5f;
    float ridgeHeight = 4.0f;
    auto ridgeBounds = juce::Rectangle<float>(
        cx - ridgeWidth / 2, cy - ridgeHeight / 2 + pressOffset,
        ridgeWidth, ridgeHeight);
    g.setColour(isOn ? juce::Colour(0xff303030) : juce::Colour(0xff4a4a4a));
    g.fillRoundedRectangle(ridgeBounds, 2.0f);

    // LED to the LEFT of footswitch
    if (ledColour.getAlpha() > 0)
    {
        float ledSize = 8.0f;
        float ledX = bounds.getX() - 18.0f;
        float ledY = cy - ledSize / 2.0f;

        if (isOn)
        {
            // Glow
            g.setColour(ledColour.withAlpha(0.4f));
            g.fillEllipse(ledX - ledSize * 0.5f, ledY - ledSize * 0.5f, ledSize * 2.0f, ledSize * 2.0f);
            g.setColour(ledColour.withAlpha(0.6f));
            g.fillEllipse(ledX - ledSize * 0.2f, ledY - ledSize * 0.2f, ledSize * 1.4f, ledSize * 1.4f);

            // Main LED
            juce::ColourGradient ledGradient(ledColour.brighter(0.5f), ledX + ledSize/2, ledY,
                                              ledColour, ledX + ledSize/2, ledY + ledSize, true);
            g.setGradientFill(ledGradient);
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
        }
        else
        {
            g.setColour(ledColour.withAlpha(0.15f));
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
            g.setColour(ledColour.withAlpha(0.3f));
            g.drawEllipse(ledX, ledY, ledSize, ledSize, 1.0f);
        }
    }
}

//==============================================================================
// LevelMeter
//==============================================================================
void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xff151515));
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
    g.setColour(VoxColors::panelBorder);
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
    g.setColour(VoxColors::panelBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

//==============================================================================
// EQVisualizer - Synth-style with color-coded bands
//==============================================================================
float EQVisualizer::freqToX(float freq, float width) const
{
    // Log scale: 20Hz to 20kHz
    float minLog = std::log10(20.0f);
    float maxLog = std::log10(20000.0f);
    float freqLog = std::log10(juce::jmax(20.0f, freq));
    return width * (freqLog - minLog) / (maxLog - minLog);
}

float EQVisualizer::dbToY(float db, float height) const
{
    // -18dB to +18dB range
    return height * 0.5f * (1.0f - db / 18.0f);
}

float EQVisualizer::getBandMagnitudeAtFrequency(float freq, int bandIndex) const
{
    return processor.getEQBandMagnitudeAtFrequency(freq, bandIndex);
}

void EQVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background - Synth style dark (#0a0908)
    g.setColour(juce::Colour(0xff0a0908));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Grid lines - Synth style (#2a2520)
    g.setColour(juce::Colour(0xff2a2520));

    // Horizontal grid (dB)
    for (int db = -12; db <= 12; db += 6)
    {
        if (db == 0) continue;  // Zero line drawn separately
        float y = dbToY((float)db, bounds.getHeight());
        g.drawHorizontalLine((int)(bounds.getY() + y), bounds.getX() + 5, bounds.getRight() - 5);
    }

    // Zero line (brighter)
    g.setColour(juce::Colour(0xff3d3428));
    float zeroY = dbToY(0.0f, bounds.getHeight());
    g.drawHorizontalLine((int)(bounds.getY() + zeroY), bounds.getX() + 5, bounds.getRight() - 5);

    // Vertical grid (frequency) - more frequency markers
    float freqs[] = { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f };
    g.setColour(juce::Colour(0xff2a2520));
    for (float freq : freqs)
    {
        float x = freqToX(freq, bounds.getWidth());
        g.drawVerticalLine((int)(bounds.getX() + x), bounds.getY() + 5, bounds.getBottom() - 5);
    }

    // Draw frequency labels
    g.setColour(juce::Colour(0xff5d4a35));
    g.setFont(juce::FontOptions(8.0f));
    const char* freqLabels[] = { "50", "100", "200", "500", "1k", "2k", "5k", "10k" };
    for (int i = 0; i < 8; ++i)
    {
        float x = freqToX(freqs[i], bounds.getWidth());
        g.drawText(freqLabels[i], (int)(bounds.getX() + x - 12), (int)(bounds.getBottom() - 12), 24, 10, juce::Justification::centred);
    }

    // Draw dB labels
    g.drawText("+12", (int)(bounds.getX() + 2), (int)(bounds.getY() + dbToY(12.0f, bounds.getHeight()) - 5), 20, 10, juce::Justification::left);
    g.drawText("0", (int)(bounds.getX() + 2), (int)(bounds.getY() + zeroY - 5), 20, 10, juce::Justification::left);
    g.drawText("-12", (int)(bounds.getX() + 2), (int)(bounds.getY() + dbToY(-12.0f, bounds.getHeight()) - 5), 20, 10, juce::Justification::left);

    // Band colors
    const juce::Colour bandColors[] = {
        juce::Colour(colorHPF),       // HPF - Red
        juce::Colour(colorLowShelf),  // Low shelf - Orange
        juce::Colour(colorLowMid),    // Low-mid - Green
        juce::Colour(colorMid),       // Mid - Cyan
        juce::Colour(colorHighMid),   // High-mid - Magenta
        juce::Colour(colorHighShelf)  // High shelf - White
    };

    // Draw individual band curves
    for (int band = 0; band < 6; ++band)
    {
        juce::Path bandPath;
        bool pathStarted = false;

        for (float x = 0; x < bounds.getWidth(); x += 2.0f)
        {
            float freq = 20.0f * std::pow(1000.0f, x / bounds.getWidth());
            float magnitude = getBandMagnitudeAtFrequency(freq, band);
            float db = juce::Decibels::gainToDecibels(magnitude, -24.0f);
            db = juce::jlimit(-18.0f, 18.0f, db);

            float y = dbToY(db, bounds.getHeight());

            if (!pathStarted)
            {
                bandPath.startNewSubPath(bounds.getX() + x, bounds.getY() + y);
                pathStarted = true;
            }
            else
            {
                bandPath.lineTo(bounds.getX() + x, bounds.getY() + y);
            }
        }

        // Draw band curve with transparency
        g.setColour(bandColors[band].withAlpha(0.5f));
        g.strokePath(bandPath, juce::PathStrokeType(1.5f));
    }

    // Draw combined frequency response curve
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

    // Fill under combined curve with gradient
    juce::Path fillPath = responsePath;
    fillPath.lineTo(bounds.getRight(), bounds.getY() + zeroY);
    fillPath.lineTo(bounds.getX(), bounds.getY() + zeroY);
    fillPath.closeSubPath();

    juce::ColourGradient fillGradient(juce::Colours::white.withAlpha(0.15f), bounds.getX(), bounds.getY(),
                                       juce::Colours::white.withAlpha(0.02f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(fillGradient);
    g.fillPath(fillPath);

    // Draw combined curve with glow effect
    // Outer glow
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.strokePath(responsePath, juce::PathStrokeType(6.0f));
    // Inner glow
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.strokePath(responsePath, juce::PathStrokeType(4.0f));
    // Main stroke
    g.setColour(juce::Colours::white);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));

    // Border
    g.setColour(juce::Colour(0xff3d3428));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
}

//==============================================================================
// CompressorSection
//==============================================================================
CompressorSection::CompressorSection()
{
    lookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    lookAndFeel->setAccentColour(juce::Colours::white);

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

    // Footswitch-style buttons (invisible, drawn manually)
    autoReleaseButton.setClickingTogglesState(true);
    autoReleaseButton.setAlpha(0.0f);
    addAndMakeVisible(autoReleaseButton);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setAlpha(0.0f);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 14);
    slider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, VoxColors::textPrimary.withAlpha(0.85f));
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void CompressorSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4);

    // Brushed metal panel
    drawBrushedMetalTexture(g, bounds, VoxColors::panelComp);

    // Beveled edge effect
    g.setColour(VoxColors::panelComp.brighter(0.2f));
    g.drawRoundedRectangle(bounds, 10.0f, 2.0f);
    g.setColour(VoxColors::panelComp.darker(0.3f));
    g.drawRoundedRectangle(bounds.reduced(2), 8.0f, 1.0f);

    // Corner screws
    float screwSize = 8.0f;
    float screwInset = 12.0f;
    drawScrew(g, bounds.getX() + screwInset, bounds.getY() + screwInset, screwSize);
    drawScrew(g, bounds.getRight() - screwInset, bounds.getY() + screwInset, screwSize);
    drawScrew(g, bounds.getX() + screwInset, bounds.getBottom() - screwInset, screwSize);
    drawScrew(g, bounds.getRight() - screwInset, bounds.getBottom() - screwInset, screwSize);

    // Title
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("COMPRESSOR", bounds.removeFromTop(28).reduced(30, 0), juce::Justification::centredLeft);

    // Draw footswitch-style toggle buttons
    auto autoBounds = autoReleaseButton.getBounds().toFloat();
    bool autoOn = autoReleaseButton.getToggleState();
    drawFootswitch(g, autoBounds, autoOn, VoxColors::meterGreen);

    // Auto label
    g.setColour(VoxColors::textSecondary);
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.drawText("AUTO", autoBounds.getX() - 30, autoBounds.getCentreY() - 6, 28, 12, juce::Justification::right);

    auto bypassBounds = bypassButton.getBounds().toFloat();
    bool bypassOn = !bypassButton.getToggleState();  // Active when NOT bypassed
    drawFootswitch(g, bypassBounds, bypassOn, VoxColors::led);

    // Bypass label
    g.drawText("BYPASS", bypassBounds.getRight() + 4, bypassBounds.getCentreY() - 6, 40, 12, juce::Justification::left);
}

void CompressorSection::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(28); // Title

    const int knobSize = 50;
    const int labelHeight = 14;
    const int spacing = 58;  // Fixed spacing to prevent overlap

    int x = 25;
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
    grMeter.setBounds(x + 15, y + 18, getWidth() - x - 90, 22);

    // Footswitch-style buttons
    autoReleaseButton.setBounds(x + 45, y + 52, 36, 36);
    bypassButton.setBounds(getWidth() - 60, y + 52, 36, 36);
}

//==============================================================================
// DeEsserSection
//==============================================================================
DeEsserSection::DeEsserSection()
{
    lookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    lookAndFeel->setAccentColour(juce::Colours::white);

    setupSlider(frequencySlider, frequencyLabel, "FREQUENCY");
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(rangeSlider, rangeLabel, "RANGE");

    frequencySlider.setTextValueSuffix(" Hz");
    thresholdSlider.setTextValueSuffix(" dB");
    rangeSlider.setTextValueSuffix(" dB");

    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setColour(juce::Label::textColourId, VoxColors::textPrimary.withAlpha(0.85f));
    modeLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(modeLabel);

    modeSelector.addItem("Split-Band", 1);
    modeSelector.addItem("Wideband", 2);
    modeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0a0a0a));
    modeSelector.setColour(juce::ComboBox::textColourId, VoxColors::lcdGreen);
    modeSelector.setColour(juce::ComboBox::outlineColourId, VoxColors::panelBorder);
    modeSelector.setColour(juce::ComboBox::arrowColourId, VoxColors::lcdGreen);
    addAndMakeVisible(modeSelector);

    // Footswitch-style buttons (invisible, drawn manually)
    listenButton.setClickingTogglesState(true);
    listenButton.setAlpha(0.0f);
    addAndMakeVisible(listenButton);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setAlpha(0.0f);
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
    label.setColour(juce::Label::textColourId, VoxColors::textPrimary.withAlpha(0.85f));
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void DeEsserSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4);

    // Brushed metal panel (dark teal)
    drawBrushedMetalTexture(g, bounds, VoxColors::panelDeEss);

    // Beveled edge effect
    g.setColour(VoxColors::panelDeEss.brighter(0.2f));
    g.drawRoundedRectangle(bounds, 10.0f, 2.0f);
    g.setColour(VoxColors::panelDeEss.darker(0.3f));
    g.drawRoundedRectangle(bounds.reduced(2), 8.0f, 1.0f);

    // Corner screws
    float screwSize = 8.0f;
    float screwInset = 12.0f;
    drawScrew(g, bounds.getX() + screwInset, bounds.getY() + screwInset, screwSize);
    drawScrew(g, bounds.getRight() - screwInset, bounds.getY() + screwInset, screwSize);
    drawScrew(g, bounds.getX() + screwInset, bounds.getBottom() - screwInset, screwSize);
    drawScrew(g, bounds.getRight() - screwInset, bounds.getBottom() - screwInset, screwSize);

    // Title with activity LED
    auto titleArea = bounds.removeFromTop(28).reduced(30, 0);
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("DE-ESSER", titleArea, juce::Justification::centredLeft);

    // Activity LED
    float ledX = titleArea.getRight() - 15;
    float ledY = titleArea.getCentreY() - 4;
    if (isActive)
    {
        g.setColour(VoxColors::led.withAlpha(0.4f));
        g.fillEllipse(ledX - 3, ledY - 3, 14, 14);
        g.setColour(VoxColors::led);
    }
    else
    {
        g.setColour(VoxColors::led.withAlpha(0.2f));
    }
    g.fillEllipse(ledX, ledY, 8, 8);

    // Draw footswitch-style toggle buttons
    auto listenBounds = listenButton.getBounds().toFloat();
    bool listenOn = listenButton.getToggleState();
    drawFootswitch(g, listenBounds, listenOn, VoxColors::meterYellow);

    // Listen label
    g.setColour(VoxColors::textSecondary);
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.drawText("LISTEN", listenBounds.getX() - 38, listenBounds.getCentreY() - 6, 36, 12, juce::Justification::right);

    auto bypassBounds = bypassButton.getBounds().toFloat();
    bool bypassOn = !bypassButton.getToggleState();  // Active when NOT bypassed
    drawFootswitch(g, bypassBounds, bypassOn, VoxColors::led);

    // Bypass label
    g.drawText("BYPASS", bypassBounds.getRight() + 4, bypassBounds.getCentreY() - 6, 40, 12, juce::Justification::left);
}

void DeEsserSection::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(28);

    const int knobSize = 50;
    const int labelHeight = 14;
    const int spacing = 68;  // Fixed spacing to prevent overlap

    int x = 25;
    int y = bounds.getY();

    auto placeKnob = [&](juce::Slider& slider, juce::Label& label)
    {
        label.setBounds(x, y, knobSize + 10, labelHeight);
        slider.setBounds(x, y + labelHeight, knobSize, knobSize);
        x += spacing;
    };

    placeKnob(frequencySlider, frequencyLabel);
    placeKnob(thresholdSlider, thresholdLabel);
    placeKnob(rangeSlider, rangeLabel);

    // Mode selector - larger and more readable
    modeLabel.setBounds(x, y, 100, labelHeight);
    modeSelector.setBounds(x, y + labelHeight + 5, 100, 28);

    // GR Meter
    grMeter.setBounds(x + 115, y + 10, getWidth() - x - 200, 22);

    // Footswitch-style buttons
    listenButton.setBounds(x + 145, y + 52, 36, 36);
    bypassButton.setBounds(getWidth() - 60, y + 52, 36, 36);
}

//==============================================================================
// EQSection
//==============================================================================
EQSection::EQSection(VoxProcAudioProcessor& p)
    : eqVisualizer(p)
{
    lookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    lookAndFeel->setAccentColour(juce::Colours::white);

    // HPF
    setupSlider(hpfFreqSlider, hpfFreqLabel, "HPF");
    hpfFreqSlider.setTextValueSuffix(" Hz");

    hpfSlopeSelector.addItem("12 dB/oct", 1);
    hpfSlopeSelector.addItem("24 dB/oct", 2);
    hpfSlopeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0a0a0a));
    hpfSlopeSelector.setColour(juce::ComboBox::textColourId, VoxColors::lcdGreen);
    hpfSlopeSelector.setColour(juce::ComboBox::outlineColourId, VoxColors::panelBorder);
    hpfSlopeSelector.setColour(juce::ComboBox::arrowColourId, VoxColors::lcdGreen);
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

    // Footswitch-style button (invisible, drawn manually)
    bypassButton.setClickingTogglesState(true);
    bypassButton.setAlpha(0.0f);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 11);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 11);
    slider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, VoxColors::textPrimary.withAlpha(0.9f));
    label.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    addAndMakeVisible(label);
}

void EQSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4);

    // Brushed metal panel (muted gray)
    drawBrushedMetalTexture(g, bounds, VoxColors::panelEQ);

    // Beveled edge effect
    g.setColour(VoxColors::panelEQ.brighter(0.2f));
    g.drawRoundedRectangle(bounds, 10.0f, 2.0f);
    g.setColour(VoxColors::panelEQ.darker(0.3f));
    g.drawRoundedRectangle(bounds.reduced(2), 8.0f, 1.0f);

    // Corner screws
    float screwSize = 8.0f;
    float screwInset = 12.0f;
    drawScrew(g, bounds.getX() + screwInset, bounds.getY() + screwInset, screwSize);
    drawScrew(g, bounds.getRight() - screwInset, bounds.getY() + screwInset, screwSize);
    drawScrew(g, bounds.getX() + screwInset, bounds.getBottom() - screwInset, screwSize);
    drawScrew(g, bounds.getRight() - screwInset, bounds.getBottom() - screwInset, screwSize);

    // Title
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    g.drawText("EQUALIZER", bounds.removeFromTop(28).reduced(30, 0), juce::Justification::centredLeft);

    // Draw footswitch-style bypass button
    auto bypassBounds = bypassButton.getBounds().toFloat();
    bool bypassOn = !bypassButton.getToggleState();  // Active when NOT bypassed
    drawFootswitch(g, bypassBounds, bypassOn, VoxColors::led);

    // Bypass label
    g.setColour(VoxColors::textSecondary);
    g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    g.drawText("BYPASS", bypassBounds.getRight() + 4, bypassBounds.getCentreY() - 6, 40, 12, juce::Justification::left);
}

void EQSection::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(28);

    // EQ Visualizer at top - taller for better visibility
    eqVisualizer.setBounds(bounds.removeFromTop(110).reduced(20, 5));

    bounds.removeFromTop(10);

    // ALL KNOBS SAME SIZE - 55px uniform
    const int knobSize = 55;
    const int smallKnobSize = 45;  // For freq/Q secondary knobs
    const int labelHeight = 12;
    const int rowSpacing = 4;

    // Calculate available width for 6 bands + bypass button
    int availableWidth = getWidth() - 80;
    int bandWidth = availableWidth / 6;
    int startX = 15;

    int y = bounds.getY();
    int secondRowY = y + labelHeight + knobSize + rowSpacing;

    // HPF - just freq and slope
    int x = startX;
    hpfFreqLabel.setBounds(x, y, knobSize, labelHeight);
    hpfFreqSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    hpfSlopeSelector.setBounds(x, secondRowY + labelHeight, knobSize, 28);

    // Low Shelf - gain and freq stacked
    x += bandWidth;
    lowShelfGainLabel.setBounds(x, y, knobSize, labelHeight);
    lowShelfGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    lowShelfFreqLabel.setBounds(x, secondRowY, smallKnobSize, labelHeight);
    lowShelfFreqSlider.setBounds(x, secondRowY + labelHeight, smallKnobSize, smallKnobSize);

    // Low-Mid - gain, freq, Q
    x += bandWidth;
    lowMidGainLabel.setBounds(x, y, knobSize, labelHeight);
    lowMidGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    lowMidFreqLabel.setBounds(x - 2, secondRowY, smallKnobSize, labelHeight);
    lowMidFreqSlider.setBounds(x - 2, secondRowY + labelHeight, smallKnobSize, smallKnobSize);
    lowMidQLabel.setBounds(x + smallKnobSize + 2, secondRowY, smallKnobSize - 10, labelHeight);
    lowMidQSlider.setBounds(x + smallKnobSize + 2, secondRowY + labelHeight, smallKnobSize - 10, smallKnobSize - 10);

    // Mid - gain, freq, Q
    x += bandWidth;
    midGainLabel.setBounds(x, y, knobSize, labelHeight);
    midGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    midFreqLabel.setBounds(x - 2, secondRowY, smallKnobSize, labelHeight);
    midFreqSlider.setBounds(x - 2, secondRowY + labelHeight, smallKnobSize, smallKnobSize);
    midQLabel.setBounds(x + smallKnobSize + 2, secondRowY, smallKnobSize - 10, labelHeight);
    midQSlider.setBounds(x + smallKnobSize + 2, secondRowY + labelHeight, smallKnobSize - 10, smallKnobSize - 10);

    // High-Mid - gain, freq, Q
    x += bandWidth;
    highMidGainLabel.setBounds(x, y, knobSize, labelHeight);
    highMidGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    highMidFreqLabel.setBounds(x - 2, secondRowY, smallKnobSize, labelHeight);
    highMidFreqSlider.setBounds(x - 2, secondRowY + labelHeight, smallKnobSize, smallKnobSize);
    highMidQLabel.setBounds(x + smallKnobSize + 2, secondRowY, smallKnobSize - 10, labelHeight);
    highMidQSlider.setBounds(x + smallKnobSize + 2, secondRowY + labelHeight, smallKnobSize - 10, smallKnobSize - 10);

    // High Shelf - gain and freq stacked
    x += bandWidth;
    highShelfGainLabel.setBounds(x, y, knobSize, labelHeight);
    highShelfGainSlider.setBounds(x, y + labelHeight, knobSize, knobSize);
    highShelfFreqLabel.setBounds(x, secondRowY, smallKnobSize, labelHeight);
    highShelfFreqSlider.setBounds(x, secondRowY + labelHeight, smallKnobSize, smallKnobSize);

    // Footswitch-style bypass button
    int bypassY = y + knobSize / 2;
    bypassButton.setBounds(getWidth() - 55, bypassY, 40, 40);
}

//==============================================================================
// VoxProcAudioProcessorEditor
//==============================================================================
VoxProcAudioProcessorEditor::VoxProcAudioProcessorEditor(VoxProcAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), eqSection(p)
{
    globalLookAndFeel = std::make_unique<VoxProcLookAndFeel>();
    globalLookAndFeel->setAccentColour(juce::Colours::white);

    // Input/Output gain sliders - smaller for header placement
    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    inputGainSlider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    inputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    inputGainSlider.setTextValueSuffix(" dB");
    inputGainSlider.setLookAndFeel(globalLookAndFeel.get());
    addAndMakeVisible(inputGainSlider);

    inputGainLabel.setText("IN", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    inputGainLabel.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    addAndMakeVisible(inputGainLabel);

    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, VoxColors::textPrimary);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    outputGainSlider.setTextValueSuffix(" dB");
    outputGainSlider.setLookAndFeel(globalLookAndFeel.get());
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("OUT", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, VoxColors::textSecondary);
    outputGainLabel.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
    addAndMakeVisible(outputGainLabel);

    // Horizontal meters for header (like PDLBRD)
    inputMeter.setVertical(false);
    outputMeter.setVertical(false);
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

    setSize(700, 640);
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
    // Dark background (matching PDLBRD)
    g.fillAll(VoxColors::background);

    // Header bar with subtle gradient
    juce::ColourGradient headerGradient(juce::Colour(0xff252525), 0.0f, 0.0f,
                                         VoxColors::background, 0.0f, 50.0f, false);
    g.setGradientFill(headerGradient);
    g.fillRect(0, 0, getWidth(), 50);

    // Header bottom edge (metal strip)
    g.setColour(juce::Colour(0xff333333));
    g.fillRect(0, 48, getWidth(), 2);
    g.setColour(juce::Colour(0xff404040));
    g.drawLine(0, 48, (float)getWidth(), 48, 1.0f);

    // Title with slight emboss effect
    g.setColour(juce::Colour(0xff111111));
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("VOXPROC", 16, 11, 200, 30, juce::Justification::centredLeft);
    g.setColour(juce::Colours::white);
    g.drawText("VOXPROC", 15, 10, 200, 30, juce::Justification::centredLeft);

}

void VoxProcAudioProcessorEditor::resized()
{
    const int headerHeight = 50;
    const int margin = 8;
    const int meterWidth = 14;
    const int gainKnobSize = 45;

    auto bounds = getLocalBounds();
    bounds.removeFromTop(headerHeight);

    // Left side: Input meter and gain
    auto leftArea = bounds.removeFromLeft(65);
    inputGainLabel.setBounds(leftArea.getX() + 10, leftArea.getY() + 5, 45, 14);
    inputGainSlider.setBounds(leftArea.getX() + 10, leftArea.getY() + 17, gainKnobSize, gainKnobSize);
    inputMeter.setBounds(leftArea.getX() + 4, leftArea.getY() + 70, gainKnobSize, leftArea.getHeight() - 80);

    // Right side: Output meter and gain
    auto rightArea = bounds.removeFromRight(65);
    outputGainLabel.setBounds(rightArea.getX() + 10, rightArea.getY() + 5, 45, 14);
    outputGainSlider.setBounds(rightArea.getX() + 10, rightArea.getY() + 17, gainKnobSize, gainKnobSize);
    outputMeter.setBounds(rightArea.getX() + 4, rightArea.getY() + 70, gainKnobSize, rightArea.getHeight() - 80);

    // Main content area
    bounds.reduce(margin, margin);

    // Compressor section
    compressorSection.setBounds(bounds.removeFromTop(125));
    bounds.removeFromTop(margin);

    // De-esser section
    deEsserSection.setBounds(bounds.removeFromTop(125));
    bounds.removeFromTop(margin);

    // EQ section (takes remaining space)
    eqSection.setBounds(bounds);
}
