#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"

class DeEsser
{
public:
    enum Mode
    {
        SplitBand = 0,
        Wideband
    };

    DeEsser();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Parameters
    void setFrequency(float freq);           // 2kHz to 12kHz - center frequency for detection
    void setThreshold(float thresholdDb);    // -60 to 0 dB
    void setRange(float rangeDb);            // 0 to 12 dB - max reduction
    void setMode(int mode);                  // 0 = Split-band, 1 = Wideband
    void setListenMode(bool enabled);        // Solo the sibilance band
    void setBypass(bool shouldBypass);

    float getGainReduction() const { return currentGainReduction; }
    bool isActive() const { return currentGainReduction > 0.5f; }
    bool isBypassed() const { return bypassed; }

private:
    void updateFilters();
    float processSample(float input, int channel);

    // Parameters
    float frequency = 6000.0f;     // Hz
    float threshold = -20.0f;      // dB
    float range = 6.0f;            // dB
    Mode mode = SplitBand;
    bool listenMode = false;
    bool bypassed = false;

    // Filter state
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Biquad filter for detection band (bandpass)
    struct BiquadState
    {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
    };

    DSPUtils::BiquadCoeffs detectionCoeffs;
    BiquadState detectionStateL, detectionStateR;

    // High-pass and low-pass for split-band mode
    DSPUtils::BiquadCoeffs highPassCoeffs;
    DSPUtils::BiquadCoeffs lowPassCoeffs;
    BiquadState hpStateL, hpStateR;
    BiquadState lpStateL, lpStateR;

    // Envelope follower
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Gain reduction
    float currentGainReduction = 0.0f;
    float smoothedGainReduction = 0.0f;

    float processBiquad(float input, const DSPUtils::BiquadCoeffs& coeffs, BiquadState& state);
};
