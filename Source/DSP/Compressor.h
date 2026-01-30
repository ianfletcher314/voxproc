#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"

class Compressor
{
public:
    Compressor();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Parameters
    void setThreshold(float thresholdDb);      // -60 to 0 dB
    void setRatio(float ratio);                 // 1 to 20
    void setAttack(float attackMs);             // 0.1 to 100 ms
    void setRelease(float releaseMs);           // 10 to 1000 ms
    void setMakeupGain(float gainDb);           // 0 to 24 dB
    void setKnee(float kneeDb);                 // Soft knee width in dB
    void setAutoRelease(bool enabled);
    void setBypass(bool shouldBypass);

    float getGainReduction() const { return currentGainReduction; }
    bool isBypassed() const { return bypassed; }

private:
    float processSample(float inputL, float inputR, float& envelopeL, float& envelopeR);
    void updateCoefficients();
    float computeGain(float inputDb);

    // Parameters
    float threshold = -20.0f;    // dB
    float ratio = 4.0f;          // :1
    float attackMs = 10.0f;      // ms
    float releaseMs = 100.0f;    // ms
    float makeupGain = 0.0f;     // dB
    float kneeWidth = 6.0f;      // dB (soft knee)
    bool autoRelease = false;
    bool bypassed = false;

    // Coefficients (calculated from parameters)
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float makeupLinear = 1.0f;

    // State
    double currentSampleRate = 44100.0;
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;
    float currentGainReduction = 0.0f;
    float smoothedGainReduction = 0.0f;
};
