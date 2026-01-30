#include "Equalizer.h"

Equalizer::Equalizer()
{
}

void Equalizer::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    updateAllFilters();
    reset();
}

void Equalizer::reset()
{
    hpf1StateL = {};
    hpf1StateR = {};
    hpf2StateL = {};
    hpf2StateR = {};
    lowShelfStateL = {};
    lowShelfStateR = {};
    lowMidStateL = {};
    lowMidStateR = {};
    midStateL = {};
    midStateR = {};
    highMidStateL = {};
    highMidStateR = {};
    highShelfStateL = {};
    highShelfStateR = {};
}

void Equalizer::updateAllFilters()
{
    updateHPF();
    updateLowShelf();
    updateLowMid();
    updateMid();
    updateHighMid();
    updateHighShelf();
}

void Equalizer::updateHPF()
{
    // Use Butterworth Q for clean response
    hpfCoeffs1 = DSPUtils::calcHighPass(currentSampleRate, hpfFreq, 0.707f);
    hpfCoeffs2 = DSPUtils::calcHighPass(currentSampleRate, hpfFreq, 0.707f);
}

void Equalizer::updateLowShelf()
{
    lowShelfCoeffs = DSPUtils::calcLowShelf(currentSampleRate, lowShelfFreq, lowShelfGain);
}

void Equalizer::updateLowMid()
{
    lowMidCoeffs = DSPUtils::calcPeaking(currentSampleRate, lowMidFreq, lowMidGain, lowMidQ);
}

void Equalizer::updateMid()
{
    midCoeffs = DSPUtils::calcPeaking(currentSampleRate, midFreq, midGain, midQ);
}

void Equalizer::updateHighMid()
{
    highMidCoeffs = DSPUtils::calcPeaking(currentSampleRate, highMidFreq, highMidGain, highMidQ);
}

void Equalizer::updateHighShelf()
{
    highShelfCoeffs = DSPUtils::calcHighShelf(currentSampleRate, highShelfFreq, highShelfGain);
}

float Equalizer::processBiquad(float input, const DSPUtils::BiquadCoeffs& coeffs, BiquadState& state)
{
    float output = coeffs.b0 * input + coeffs.b1 * state.x1 + coeffs.b2 * state.x2
                 - coeffs.a1 * state.y1 - coeffs.a2 * state.y2;

    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = output;

    return output;
}

// Parameter setters
void Equalizer::setHPFFrequency(float freq)
{
    hpfFreq = std::clamp(freq, 20.0f, 400.0f);
    updateHPF();
}

void Equalizer::setHPFSlope(int slope)
{
    hpfSlope = (slope >= 24) ? 24 : 12;
}

void Equalizer::setLowShelfFrequency(float freq)
{
    lowShelfFreq = std::clamp(freq, 50.0f, 500.0f);
    updateLowShelf();
}

void Equalizer::setLowShelfGain(float gainDb)
{
    lowShelfGain = std::clamp(gainDb, -12.0f, 12.0f);
    updateLowShelf();
}

void Equalizer::setLowMidFrequency(float freq)
{
    lowMidFreq = std::clamp(freq, 100.0f, 1000.0f);
    updateLowMid();
}

void Equalizer::setLowMidGain(float gainDb)
{
    lowMidGain = std::clamp(gainDb, -12.0f, 12.0f);
    updateLowMid();
}

void Equalizer::setLowMidQ(float q)
{
    lowMidQ = std::clamp(q, 0.5f, 10.0f);
    updateLowMid();
}

void Equalizer::setMidFrequency(float freq)
{
    midFreq = std::clamp(freq, 500.0f, 4000.0f);
    updateMid();
}

void Equalizer::setMidGain(float gainDb)
{
    midGain = std::clamp(gainDb, -12.0f, 12.0f);
    updateMid();
}

void Equalizer::setMidQ(float q)
{
    midQ = std::clamp(q, 0.5f, 10.0f);
    updateMid();
}

void Equalizer::setHighMidFrequency(float freq)
{
    highMidFreq = std::clamp(freq, 2000.0f, 8000.0f);
    updateHighMid();
}

void Equalizer::setHighMidGain(float gainDb)
{
    highMidGain = std::clamp(gainDb, -12.0f, 12.0f);
    updateHighMid();
}

void Equalizer::setHighMidQ(float q)
{
    highMidQ = std::clamp(q, 0.5f, 10.0f);
    updateHighMid();
}

void Equalizer::setHighShelfFrequency(float freq)
{
    highShelfFreq = std::clamp(freq, 4000.0f, 16000.0f);
    updateHighShelf();
}

void Equalizer::setHighShelfGain(float gainDb)
{
    highShelfGain = std::clamp(gainDb, -12.0f, 12.0f);
    updateHighShelf();
}

void Equalizer::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}

float Equalizer::getMagnitudeAtFrequency(float freq) const
{
    // Calculate combined magnitude response at a given frequency
    // This is used for the EQ visualization

    float w = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(currentSampleRate);
    float cosw = std::cos(w);
    float cos2w = std::cos(2.0f * w);
    float sinw = std::sin(w);
    float sin2w = std::sin(2.0f * w);

    auto calcMagnitude = [&](const DSPUtils::BiquadCoeffs& c) -> float
    {
        float numReal = c.b0 + c.b1 * cosw + c.b2 * cos2w;
        float numImag = -c.b1 * sinw - c.b2 * sin2w;
        float denReal = 1.0f + c.a1 * cosw + c.a2 * cos2w;
        float denImag = -c.a1 * sinw - c.a2 * sin2w;

        float numMag = std::sqrt(numReal * numReal + numImag * numImag);
        float denMag = std::sqrt(denReal * denReal + denImag * denImag);

        return numMag / denMag;
    };

    float magnitude = 1.0f;

    // HPF (apply once or twice depending on slope)
    if (hpfFreq > 20.0f)
    {
        magnitude *= calcMagnitude(hpfCoeffs1);
        if (hpfSlope >= 24)
            magnitude *= calcMagnitude(hpfCoeffs2);
    }

    // All other bands
    if (std::abs(lowShelfGain) > 0.1f)
        magnitude *= calcMagnitude(lowShelfCoeffs);

    if (std::abs(lowMidGain) > 0.1f)
        magnitude *= calcMagnitude(lowMidCoeffs);

    if (std::abs(midGain) > 0.1f)
        magnitude *= calcMagnitude(midCoeffs);

    if (std::abs(highMidGain) > 0.1f)
        magnitude *= calcMagnitude(highMidCoeffs);

    if (std::abs(highShelfGain) > 0.1f)
        magnitude *= calcMagnitude(highShelfCoeffs);

    return magnitude;
}

float Equalizer::getBandMagnitudeAtFrequency(float freq, int bandIndex) const
{
    // Calculate magnitude for a single EQ band
    float w = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(currentSampleRate);
    float cosw = std::cos(w);
    float cos2w = std::cos(2.0f * w);
    float sinw = std::sin(w);
    float sin2w = std::sin(2.0f * w);

    auto calcMagnitude = [&](const DSPUtils::BiquadCoeffs& c) -> float
    {
        float numReal = c.b0 + c.b1 * cosw + c.b2 * cos2w;
        float numImag = -c.b1 * sinw - c.b2 * sin2w;
        float denReal = 1.0f + c.a1 * cosw + c.a2 * cos2w;
        float denImag = -c.a1 * sinw - c.a2 * sin2w;

        float numMag = std::sqrt(numReal * numReal + numImag * numImag);
        float denMag = std::sqrt(denReal * denReal + denImag * denImag);

        return numMag / denMag;
    };

    switch (bandIndex)
    {
        case HPF:
            if (hpfFreq > 20.0f)
            {
                float mag = calcMagnitude(hpfCoeffs1);
                if (hpfSlope >= 24)
                    mag *= calcMagnitude(hpfCoeffs2);
                return mag;
            }
            return 1.0f;

        case LowShelf:
            return calcMagnitude(lowShelfCoeffs);

        case LowMid:
            return calcMagnitude(lowMidCoeffs);

        case Mid:
            return calcMagnitude(midCoeffs);

        case HighMid:
            return calcMagnitude(highMidCoeffs);

        case HighShelf:
            return calcMagnitude(highShelfCoeffs);

        default:
            return 1.0f;
    }
}

void Equalizer::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0)
        return;

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        float sampleL = leftChannel[i];
        float sampleR = rightChannel ? rightChannel[i] : sampleL;

        // HPF (if enabled - freq > 20Hz)
        if (hpfFreq > 20.0f)
        {
            sampleL = processBiquad(sampleL, hpfCoeffs1, hpf1StateL);
            sampleR = processBiquad(sampleR, hpfCoeffs1, hpf1StateR);

            if (hpfSlope >= 24)
            {
                sampleL = processBiquad(sampleL, hpfCoeffs2, hpf2StateL);
                sampleR = processBiquad(sampleR, hpfCoeffs2, hpf2StateR);
            }
        }

        // Low Shelf
        if (std::abs(lowShelfGain) > 0.1f)
        {
            sampleL = processBiquad(sampleL, lowShelfCoeffs, lowShelfStateL);
            sampleR = processBiquad(sampleR, lowShelfCoeffs, lowShelfStateR);
        }

        // Low-Mid Parametric
        if (std::abs(lowMidGain) > 0.1f)
        {
            sampleL = processBiquad(sampleL, lowMidCoeffs, lowMidStateL);
            sampleR = processBiquad(sampleR, lowMidCoeffs, lowMidStateR);
        }

        // Mid Parametric
        if (std::abs(midGain) > 0.1f)
        {
            sampleL = processBiquad(sampleL, midCoeffs, midStateL);
            sampleR = processBiquad(sampleR, midCoeffs, midStateR);
        }

        // High-Mid Parametric
        if (std::abs(highMidGain) > 0.1f)
        {
            sampleL = processBiquad(sampleL, highMidCoeffs, highMidStateL);
            sampleR = processBiquad(sampleR, highMidCoeffs, highMidStateR);
        }

        // High Shelf
        if (std::abs(highShelfGain) > 0.1f)
        {
            sampleL = processBiquad(sampleL, highShelfCoeffs, highShelfStateL);
            sampleR = processBiquad(sampleR, highShelfCoeffs, highShelfStateR);
        }

        leftChannel[i] = sampleL;
        if (rightChannel)
            rightChannel[i] = sampleR;
    }
}
