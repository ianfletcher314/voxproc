#pragma once

#include <cmath>

namespace DSPUtils
{
    inline float linearToDecibels(float linear)
    {
        return linear > 0.0f ? 20.0f * std::log10(linear) : -100.0f;
    }

    inline float decibelsToLinear(float dB)
    {
        return std::pow(10.0f, dB / 20.0f);
    }

    inline float mapRange(float value, float inMin, float inMax, float outMin, float outMax)
    {
        return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
    }

    inline float softClip(float sample)
    {
        return std::tanh(sample);
    }

    inline float hardClip(float sample, float threshold = 1.0f)
    {
        return std::clamp(sample, -threshold, threshold);
    }

    // Calculate one-pole filter coefficient for given time constant
    inline float calculateCoefficient(double sampleRate, float timeMs)
    {
        if (timeMs <= 0.0f) return 1.0f;
        return 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
    }

    // Calculate biquad coefficients for various filter types
    struct BiquadCoeffs
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    inline BiquadCoeffs calcHighPass(double sampleRate, float freq, float q = 0.707f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f + cosw0) / 2.0f) / a0;
        c.b1 = -(1.0f + cosw0) / a0;
        c.b2 = ((1.0f + cosw0) / 2.0f) / a0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calcLowPass(double sampleRate, float freq, float q = 0.707f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f - cosw0) / 2.0f) / a0;
        c.b1 = (1.0f - cosw0) / a0;
        c.b2 = ((1.0f - cosw0) / 2.0f) / a0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calcBandPass(double sampleRate, float freq, float q = 1.0f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = alpha / a0;
        c.b1 = 0.0f;
        c.b2 = -alpha / a0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calcPeaking(double sampleRate, float freq, float gainDb, float q = 1.0f)
    {
        BiquadCoeffs c;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha / A;
        c.b0 = (1.0f + alpha * A) / a0;
        c.b1 = (-2.0f * cosw0) / a0;
        c.b2 = (1.0f - alpha * A) / a0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha / A) / a0;
        return c;
    }

    inline BiquadCoeffs calcLowShelf(double sampleRate, float freq, float gainDb, float slope = 1.0f)
    {
        BiquadCoeffs c;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / 2.0f * std::sqrt((A + 1.0f / A) * (1.0f / slope - 1.0f) + 2.0f);
        float sqrtA2alpha = 2.0f * std::sqrt(A) * alpha;

        float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + sqrtA2alpha;
        c.b0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + sqrtA2alpha)) / a0;
        c.b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
        c.b2 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 - sqrtA2alpha)) / a0;
        c.a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
        c.a2 = ((A + 1.0f) + (A - 1.0f) * cosw0 - sqrtA2alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calcHighShelf(double sampleRate, float freq, float gainDb, float slope = 1.0f)
    {
        BiquadCoeffs c;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / 2.0f * std::sqrt((A + 1.0f / A) * (1.0f / slope - 1.0f) + 2.0f);
        float sqrtA2alpha = 2.0f * std::sqrt(A) * alpha;

        float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + sqrtA2alpha;
        c.b0 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 + sqrtA2alpha)) / a0;
        c.b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
        c.b2 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 - sqrtA2alpha)) / a0;
        c.a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
        c.a2 = ((A + 1.0f) - (A - 1.0f) * cosw0 - sqrtA2alpha) / a0;
        return c;
    }
}
