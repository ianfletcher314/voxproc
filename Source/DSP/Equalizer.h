#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"

class Equalizer
{
public:
    // Band indices
    enum Band
    {
        HPF = 0,
        LowShelf,
        LowMid,
        Mid,
        HighMid,
        HighShelf,
        NumBands
    };

    Equalizer();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // HPF parameters
    void setHPFFrequency(float freq);        // 20Hz to 400Hz
    void setHPFSlope(int slope);             // 12 or 24 dB/oct

    // Low Shelf parameters
    void setLowShelfFrequency(float freq);   // 50Hz to 500Hz
    void setLowShelfGain(float gainDb);      // -12 to +12 dB

    // Low-Mid Band (parametric)
    void setLowMidFrequency(float freq);     // 100Hz to 1kHz
    void setLowMidGain(float gainDb);        // -12 to +12 dB
    void setLowMidQ(float q);                // 0.5 to 10

    // Mid Band (parametric)
    void setMidFrequency(float freq);        // 500Hz to 4kHz
    void setMidGain(float gainDb);           // -12 to +12 dB
    void setMidQ(float q);                   // 0.5 to 10

    // High-Mid Band (parametric)
    void setHighMidFrequency(float freq);    // 2kHz to 8kHz
    void setHighMidGain(float gainDb);       // -12 to +12 dB
    void setHighMidQ(float q);               // 0.5 to 10

    // High Shelf parameters
    void setHighShelfFrequency(float freq);  // 4kHz to 16kHz
    void setHighShelfGain(float gainDb);     // -12 to +12 dB

    void setBypass(bool shouldBypass);
    bool isBypassed() const { return bypassed; }

    // Get frequency response for visualization (returns magnitude at given frequency)
    float getMagnitudeAtFrequency(float freq) const;
    float getBandMagnitudeAtFrequency(float freq, int bandIndex) const;

private:
    void updateAllFilters();
    void updateHPF();
    void updateLowShelf();
    void updateLowMid();
    void updateMid();
    void updateHighMid();
    void updateHighShelf();

    struct BiquadState
    {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
    };

    float processBiquad(float input, const DSPUtils::BiquadCoeffs& coeffs, BiquadState& state);

    // Parameters
    // HPF
    float hpfFreq = 80.0f;
    int hpfSlope = 12;  // 12 or 24 dB/oct

    // Low Shelf
    float lowShelfFreq = 200.0f;
    float lowShelfGain = 0.0f;

    // Low-Mid (parametric)
    float lowMidFreq = 400.0f;
    float lowMidGain = 0.0f;
    float lowMidQ = 1.0f;

    // Mid (parametric)
    float midFreq = 1000.0f;
    float midGain = 0.0f;
    float midQ = 1.0f;

    // High-Mid (parametric)
    float highMidFreq = 4000.0f;
    float highMidGain = 0.0f;
    float highMidQ = 1.0f;

    // High Shelf
    float highShelfFreq = 8000.0f;
    float highShelfGain = 0.0f;

    bool bypassed = false;

    // Coefficients
    double currentSampleRate = 44100.0;
    DSPUtils::BiquadCoeffs hpfCoeffs1;
    DSPUtils::BiquadCoeffs hpfCoeffs2;  // Second stage for 24dB slope
    DSPUtils::BiquadCoeffs lowShelfCoeffs;
    DSPUtils::BiquadCoeffs lowMidCoeffs;
    DSPUtils::BiquadCoeffs midCoeffs;
    DSPUtils::BiquadCoeffs highMidCoeffs;
    DSPUtils::BiquadCoeffs highShelfCoeffs;

    // State (stereo)
    BiquadState hpf1StateL, hpf1StateR;
    BiquadState hpf2StateL, hpf2StateR;
    BiquadState lowShelfStateL, lowShelfStateR;
    BiquadState lowMidStateL, lowMidStateR;
    BiquadState midStateL, midStateR;
    BiquadState highMidStateL, highMidStateR;
    BiquadState highShelfStateL, highShelfStateR;
};
