#include "DeEsser.h"

DeEsser::DeEsser()
{
}

void DeEsser::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Fast attack, medium release for sibilance detection
    attackCoeff = DSPUtils::calculateCoefficient(sampleRate, 0.5f);   // 0.5ms attack
    releaseCoeff = DSPUtils::calculateCoefficient(sampleRate, 50.0f); // 50ms release

    updateFilters();
    reset();
}

void DeEsser::reset()
{
    detectionStateL = {};
    detectionStateR = {};
    hpStateL = {};
    hpStateR = {};
    lpStateL = {};
    lpStateR = {};
    envelopeL = 0.0f;
    envelopeR = 0.0f;
    currentGainReduction = 0.0f;
    smoothedGainReduction = 0.0f;
    smoothedGain = 1.0f;
}

void DeEsser::updateFilters()
{
    // Detection bandpass filter centered on sibilance frequency
    // Use relatively narrow Q for precise detection
    detectionCoeffs = DSPUtils::calcBandPass(currentSampleRate, frequency, 2.0f);

    // For split-band mode: high-pass and low-pass at the crossover frequency
    highPassCoeffs = DSPUtils::calcHighPass(currentSampleRate, frequency * 0.8f, 0.707f);
    lowPassCoeffs = DSPUtils::calcLowPass(currentSampleRate, frequency * 0.8f, 0.707f);
}

float DeEsser::processBiquad(float input, const DSPUtils::BiquadCoeffs& coeffs, BiquadState& state)
{
    float output = coeffs.b0 * input + coeffs.b1 * state.x1 + coeffs.b2 * state.x2
                 - coeffs.a1 * state.y1 - coeffs.a2 * state.y2;

    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = output;

    return output;
}

void DeEsser::setFrequency(float freq)
{
    frequency = std::clamp(freq, 2000.0f, 12000.0f);
    updateFilters();
}

void DeEsser::setThreshold(float thresholdDb)
{
    threshold = std::clamp(thresholdDb, -60.0f, 0.0f);
}

void DeEsser::setRange(float rangeDb)
{
    range = std::clamp(rangeDb, 0.0f, 12.0f);
}

void DeEsser::setMode(int modeValue)
{
    mode = static_cast<Mode>(std::clamp(modeValue, 0, 1));
}

void DeEsser::setListenMode(bool enabled)
{
    listenMode = enabled;
}

void DeEsser::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}

void DeEsser::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0)
        return;

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    float maxGR = 0.0f;
    float thresholdLinear = DSPUtils::decibelsToLinear(threshold);

    // Smoothing coefficient for gain changes (prevents clicks)
    float gainSmoothCoeff = DSPUtils::calculateCoefficient(currentSampleRate, 2.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = leftChannel[i];
        float inR = rightChannel ? rightChannel[i] : inL;

        // Run detection filter on input
        float detectedL = processBiquad(inL, detectionCoeffs, detectionStateL);
        float detectedR = processBiquad(inR, detectionCoeffs, detectionStateR);

        // Envelope follower for detection
        float detectedLevel = std::max(std::abs(detectedL), std::abs(detectedR));

        if (detectedLevel > envelopeL)
            envelopeL += attackCoeff * (detectedLevel - envelopeL);
        else
            envelopeL += releaseCoeff * (detectedLevel - envelopeL);

        envelopeR = envelopeL; // Linked stereo

        // Calculate gain reduction
        float gainReductionDb = 0.0f;
        if (envelopeL > thresholdLinear)
        {
            float overDb = DSPUtils::linearToDecibels(envelopeL) - threshold;
            gainReductionDb = std::min(overDb, range);
        }
        maxGR = std::max(maxGR, gainReductionDb);

        // Smooth the gain to prevent clicks
        float targetGain = DSPUtils::decibelsToLinear(-gainReductionDb);
        smoothedGain += gainSmoothCoeff * (targetGain - smoothedGain);

        if (listenMode)
        {
            // Output only the detected sibilance band
            leftChannel[i] = detectedL;
            if (rightChannel)
                rightChannel[i] = detectedR;
        }
        else if (mode == SplitBand)
        {
            // Split-band mode: only reduce gain in the high frequency band
            // But crossfade with dry signal to avoid phase artifacts when not de-essing
            float lowL = processBiquad(inL, lowPassCoeffs, lpStateL);
            float highL = processBiquad(inL, highPassCoeffs, hpStateL);
            float processedL = lowL + highL * smoothedGain;

            // Crossfade: when gain is 1.0 (no reduction), use dry signal
            // When gain < 1.0, blend toward processed signal
            float wetAmount = 1.0f - smoothedGain;  // 0 when no reduction, approaches 1 with more reduction
            leftChannel[i] = inL * (1.0f - wetAmount) + processedL * wetAmount;

            if (rightChannel)
            {
                float lowR = processBiquad(inR, lowPassCoeffs, lpStateR);
                float highR = processBiquad(inR, highPassCoeffs, hpStateR);
                float processedR = lowR + highR * smoothedGain;
                rightChannel[i] = inR * (1.0f - wetAmount) + processedR * wetAmount;
            }
        }
        else
        {
            // Wideband mode: reduce gain of entire signal (no coloration when not active)
            leftChannel[i] = inL * smoothedGain;
            if (rightChannel)
                rightChannel[i] = inR * smoothedGain;
        }
    }

    // Smooth the gain reduction for metering
    smoothedGainReduction = smoothedGainReduction * 0.85f + maxGR * 0.15f;
    currentGainReduction = smoothedGainReduction;
}
