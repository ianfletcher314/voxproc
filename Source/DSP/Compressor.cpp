#include "Compressor.h"

Compressor::Compressor()
{
}

void Compressor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    updateCoefficients();
    reset();
}

void Compressor::reset()
{
    envelopeL = 0.0f;
    envelopeR = 0.0f;
    currentGainReduction = 0.0f;
    smoothedGainReduction = 0.0f;
}

void Compressor::updateCoefficients()
{
    attackCoeff = DSPUtils::calculateCoefficient(currentSampleRate, attackMs);
    releaseCoeff = DSPUtils::calculateCoefficient(currentSampleRate, releaseMs);
    makeupLinear = DSPUtils::decibelsToLinear(makeupGain);
}

void Compressor::setThreshold(float thresholdDb)
{
    threshold = std::clamp(thresholdDb, -60.0f, 0.0f);
}

void Compressor::setRatio(float newRatio)
{
    ratio = std::clamp(newRatio, 1.0f, 20.0f);
}

void Compressor::setAttack(float newAttackMs)
{
    attackMs = std::clamp(newAttackMs, 0.1f, 100.0f);
    updateCoefficients();
}

void Compressor::setRelease(float newReleaseMs)
{
    releaseMs = std::clamp(newReleaseMs, 10.0f, 1000.0f);
    updateCoefficients();
}

void Compressor::setMakeupGain(float gainDb)
{
    makeupGain = std::clamp(gainDb, 0.0f, 24.0f);
    makeupLinear = DSPUtils::decibelsToLinear(makeupGain);
}

void Compressor::setKnee(float kneeDb)
{
    kneeWidth = std::clamp(kneeDb, 0.0f, 12.0f);
}

void Compressor::setAutoRelease(bool enabled)
{
    autoRelease = enabled;
}

void Compressor::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}

float Compressor::computeGain(float inputDb)
{
    // Soft knee compression
    float gainReductionDb = 0.0f;

    if (kneeWidth > 0.0f)
    {
        // Soft knee region
        float kneeStart = threshold - kneeWidth / 2.0f;
        float kneeEnd = threshold + kneeWidth / 2.0f;

        if (inputDb <= kneeStart)
        {
            // Below knee - no compression
            gainReductionDb = 0.0f;
        }
        else if (inputDb >= kneeEnd)
        {
            // Above knee - full compression
            float overDb = inputDb - threshold;
            gainReductionDb = overDb * (1.0f - 1.0f / ratio);
        }
        else
        {
            // In knee region - interpolate
            float kneeProgress = (inputDb - kneeStart) / kneeWidth;
            float softRatio = 1.0f + (ratio - 1.0f) * kneeProgress;
            float overDb = inputDb - kneeStart;
            gainReductionDb = overDb * (1.0f - 1.0f / softRatio) * kneeProgress;
        }
    }
    else
    {
        // Hard knee
        if (inputDb > threshold)
        {
            float overDb = inputDb - threshold;
            gainReductionDb = overDb * (1.0f - 1.0f / ratio);
        }
    }

    return gainReductionDb;
}

float Compressor::processSample(float inputL, float inputR, float& envL, float& envR)
{
    // Get input level (max of stereo channels for linked compression)
    float inputLevel = std::max(std::abs(inputL), std::abs(inputR));

    // Envelope follower with attack/release
    if (inputLevel > envL)
        envL += attackCoeff * (inputLevel - envL);
    else
        envL += releaseCoeff * (inputLevel - envL);

    // Same for right channel (keeping in sync for stereo)
    envR = envL;

    // Convert to dB for gain calculation
    float inputDb = DSPUtils::linearToDecibels(envL);

    // Calculate gain reduction
    float gainReductionDb = computeGain(inputDb);

    // Auto-release adjustment based on gain reduction amount
    if (autoRelease && gainReductionDb > 6.0f)
    {
        // Increase release time for heavy compression
        float autoReleaseCoeff = DSPUtils::calculateCoefficient(currentSampleRate, releaseMs * 2.0f);
        envL += (autoReleaseCoeff - releaseCoeff) * 0.5f * (inputLevel - envL);
    }

    return gainReductionDb;
}

void Compressor::process(juce::AudioBuffer<float>& buffer)
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

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = leftChannel[i];
        float inR = rightChannel ? rightChannel[i] : inL;

        float gainReductionDb = processSample(inL, inR, envelopeL, envelopeR);
        maxGR = std::max(maxGR, gainReductionDb);

        // Convert gain reduction to linear
        float gainLinear = DSPUtils::decibelsToLinear(-gainReductionDb);

        // Apply compression with makeup gain
        leftChannel[i] = inL * gainLinear * makeupLinear;
        if (rightChannel)
            rightChannel[i] = inR * gainLinear * makeupLinear;
    }

    // Smooth the gain reduction for metering
    smoothedGainReduction = smoothedGainReduction * 0.9f + maxGR * 0.1f;
    currentGainReduction = smoothedGainReduction;
}
