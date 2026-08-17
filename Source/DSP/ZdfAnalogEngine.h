#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <atomic>

namespace rackstrip
{
namespace dsp
{

// =============================================================================
// Zero-Delay Feedback (ZDF) Trapezoidal State-Variable Filter (TPT / SVF)
// Zero Latency, Artifact-Free Modulation, No Nyquist Cramping
// =============================================================================
class ZdfSvf
{
public:
    enum class Mode
    {
        LowPass,
        HighPass,
        BandPass,
        Peak,
        LowShelf,
        HighShelf,
        BellProportional
    };

    ZdfSvf() = default;

    void prepare(double sampleRate)
    {
        fs = sampleRate;
        reset();
    }

    void reset()
    {
        s1[0] = s1[1] = 0.0f;
        s2[0] = s2[1] = 0.0f;
    }

    void update(float frequencyHz, float q, float gainDb, Mode filterMode)
    {
        mode = filterMode;

        // Clamp frequency safely below Nyquist
        float fClamped = juce::jlimit(10.0f, static_cast<float>(fs * 0.495), frequencyHz);
        float w = juce::MathConstants<float>::pi * fClamped / static_cast<float>(fs);
        
        // Trapezoidal integrator pre-warping
        g = std::tan(w);

        // sqrt(gain linear) for shelf & peak
        A = std::pow(10.0f, gainDb / 40.0f);

        float currentQ = juce::jmax(0.1f, q);
        if (mode == Mode::BellProportional)
            currentQ += std::abs(gainDb) * 0.08f; // Proportional Q bandwidth narrowing

        k = 1.0f / currentQ; // Damping
    }

    inline float processSample(int channel, float in) noexcept
    {
        // ZDF trapezoidal resolution
        float D = 1.0f / (1.0f + g * (g + k));

        float ic1 = s1[channel];
        float ic2 = s2[channel];

        // High-pass node
        float hp = (in - (g + k) * ic1 - ic2) * D;

        // Band-pass node
        float bp = g * hp + ic1;

        // Low-pass node
        float lp = g * bp + ic2;

        // Exact trapezoidal state updates
        s1[channel] = 2.0f * bp - ic1;
        s2[channel] = 2.0f * lp - ic2;

        switch (mode)
        {
            case Mode::HighPass:
                return hp;
            case Mode::LowPass:
                return lp;
            case Mode::BandPass:
                return bp;
            case Mode::Peak:
            case Mode::BellProportional:
                // Symmetrical analog boost / cut
                return in + (A * A - 1.0f) * k * bp;
            case Mode::LowShelf:
                return in + (A * A - 1.0f) * lp;
            case Mode::HighShelf:
                return in + (A * A - 1.0f) * hp;
            default:
                return in;
        }
    }

private:
    double fs = 44100.0;
    Mode mode = Mode::Peak;
    float g = 0.0f;
    float k = 1.0f;
    float A = 1.0f;
    float s1[2] = { 0.0f, 0.0f };
    float s2[2] = { 0.0f, 0.0f };
};

// =============================================================================
// Transformer Core Saturation Model (Preamp Drive & Output Stages)
// Frequency-Weighted Hysteresis & Algebraic Sigmoid Waveshaper
// =============================================================================
class TransformerStage
{
public:
    TransformerStage() = default;

    void prepare(double sampleRate)
    {
        fs = sampleRate;
        reset();
    }

    void reset()
    {
        dcBlocker[0] = dcBlocker[1] = 0.0f;
        prevIn[0] = prevIn[1] = 0.0f;
    }

    inline float processSample(int channel, float in, float driveLinear) noexcept
    {
        float x = in * driveLinear;

        // Asymmetrical core flux weighting (generates 2nd and 3rd harmonics)
        float asym = 0.12f * x * std::abs(x);
        float x_drive = x + asym;
        float x2 = x_drive * x_drive;

        // Algebraic sigmoid soft-clipper: f(x) = x / sqrt(1 + x^2 + 0.2*x^4)
        float sat = x_drive / std::sqrt(1.0f + x2 + 0.2f * x2 * x2);

        // 5 Hz DC-Blocker (0-sample latency)
        float r = 0.9992f;
        float out = sat - prevIn[channel] + r * dcBlocker[channel];
        prevIn[channel] = sat;
        dcBlocker[channel] = out;

        return out;
    }

private:
    double fs = 44100.0;
    float dcBlocker[2] = { 0.0f, 0.0f };
    float prevIn[2] = { 0.0f, 0.0f };
};

// =============================================================================
// State-Space 1176 FET Compressor Engine
// Dynamic Variable Resistance Shunt Attenuation with Peak Detection
// =============================================================================
class Fet1176Engine
{
public:
    Fet1176Engine() = default;

    void prepare(double sampleRate)
    {
        fs = sampleRate;
        envState = 0.0f;
    }

    void reset()
    {
        envState = 0.0f;
    }

    inline void processBlock(juce::AudioBuffer<float>& buffer,
                             float threshDb,
                             float ratio,
                             float attackMs,
                             float releaseMs,
                             bool allButtons,
                             std::atomic<float>& grMeterDb) noexcept
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        float effectiveRatio = allButtons ? 20.0f : ratio;
        float effectiveAttackMs = allButtons ? 0.02f : juce::jlimit(0.02f, 10.0f, attackMs);
        float effectiveReleaseMs = allButtons ? (releaseMs * 0.35f) : releaseMs;

        float attackCoef = std::exp(-1.0f / (0.001f * effectiveAttackMs * static_cast<float>(fs)));
        float releaseCoef = std::exp(-1.0f / (0.001f * effectiveReleaseMs * static_cast<float>(fs)));

        float maxGrDb = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            // Instantaneous peak detection across channels
            float peak = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                peak = std::max(peak, std::abs(buffer.getSample(ch, i)));

            // Envelope charging
            float coef = (peak > envState) ? attackCoef : releaseCoef;
            envState = coef * envState + (1.0f - coef) * peak;

            float envDb = juce::Decibels::gainToDecibels(envState + 1e-6f);
            float grDb = 0.0f;

            if (envDb > threshDb)
            {
                grDb = (envDb - threshDb) * (1.0f - 1.0f / effectiveRatio);
                if (allButtons)
                {
                    // 1176 All-Buttons multi-slope knee expansion
                    grDb *= 1.25f;
                }
            }

            maxGrDb = std::max(maxGrDb, grDb);

            // Shunt FET variable resistance attenuation
            float linAtten = juce::Decibels::decibelsToGain(-grDb);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float s = buffer.getSample(ch, i) * linAtten;
                if (allButtons)
                {
                    // All-buttons FET saturation & punch
                    s = std::tanh(s * 1.35f) * 0.74f;
                }
                buffer.setSample(ch, i, s);
            }
        }

        grMeterDb.store(maxGrDb, std::memory_order_relaxed);
    }

private:
    double fs = 44100.0;
    float envState = 0.0f;
};

} // namespace dsp
} // namespace rackstrip
