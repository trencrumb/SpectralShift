//
// Created by Kyle Ramsey on 15/12/2025.
//

#pragma once
#include "EnvelopeFollower.h"
#include <algorithm>
#include <cmath>

class TransientShaper
{
public:
    enum class SmoothingMode
    {
        Sharp,   // Aggressive transient shaping, fast response
        Medium,  // Balanced - good for most uses
        Smooth   // Gentle, minimal artifacts
    };

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        fastEnv.prepare(sampleRate);
        slowEnv.prepare(sampleRate);

        // Will be set by setSmoothingMode()
        smoothedGain = 1.0f;

        // Default to Medium
        setSmoothingMode(SmoothingMode::Medium);
    }

    void setSmoothingMode(SmoothingMode mode)
    {
        currentMode = mode;

        switch (mode)
        {
            case SmoothingMode::Sharp:
                // Fast, aggressive transient detection
                fastEnv.setAttackRelease(0.5f, 30.0f);
                slowEnv.setAttackRelease(50.0f, 200.0f);
                gainSmoothCoeff = std::exp(-1.0f / (0.005f * sr)); // 5ms smoothing
                transientThreshold = 0.3f; // More sensitive
                break;

            case SmoothingMode::Medium:
                // Balanced - good for most material
                fastEnv.setAttackRelease(1.0f, 50.0f);
                slowEnv.setAttackRelease(100.0f, 300.0f);
                gainSmoothCoeff = std::exp(-1.0f / (0.01f * sr)); // 10ms smoothing
                transientThreshold = 0.5f; // Moderate sensitivity
                break;

            case SmoothingMode::Smooth:
                // Gentle, minimal artifacts
                fastEnv.setAttackRelease(5.0f, 100.0f);
                slowEnv.setAttackRelease(200.0f, 500.0f);
                gainSmoothCoeff = std::exp(-1.0f / (0.02f * sr)); // 20ms smoothing
                transientThreshold = 0.7f; // Less sensitive
                break;
        }
    }

    void setParameters(float attackDB, float sustainDB)
    {
        // Clamp to -15 to +15 dB range
        attackGainDB  = std::clamp(attackDB, -15.0f, 15.0f);
        sustainGainDB = std::clamp(sustainDB, -15.0f, 15.0f);

        attackGain  = dBToGain(attackGainDB);
        sustainGain = dBToGain(sustainGainDB);
    }

    float processSample(float x)
    {
        float inputAbs = std::abs(x);

        // Envelope following
        float fast = fastEnv.process(inputAbs);
        float slow = slowEnv.process(inputAbs);

        // Transient detection
        float ratio = fast / (slow + 0.0001f);

        // Map ratio to 0-1 range using the mode's threshold
        float transientAmount = std::clamp((ratio - 1.0f) / transientThreshold, 0.0f, 1.0f);

        // Smooth the transient curve
        transientAmount = transientAmount * transientAmount * (3.0f - 2.0f * transientAmount);

        // Calculate target gain
        float targetGain = transientAmount * attackGain + (1.0f - transientAmount) * sustainGain;

        // Smooth the gain
        smoothedGain = gainSmoothCoeff * smoothedGain + (1.0f - gainSmoothCoeff) * targetGain;

        return x * smoothedGain;
    }

    // For debugging/visualization
    float getTransientAmount() const { return lastTransientAmount; }
    float getCurrentGain() const { return smoothedGain; }
    SmoothingMode getCurrentMode() const { return currentMode; }

private:
    EnvelopeFollower fastEnv, slowEnv;

    double sr = 44100.0;
    float attackGainDB  = 0.0f;
    float sustainGainDB = 0.0f;
    float attackGain    = 1.0f;
    float sustainGain   = 1.0f;

    float smoothedGain = 1.0f;
    float gainSmoothCoeff = 0.99f;
    float transientThreshold = 0.5f;
    float lastTransientAmount = 0.0f;

    SmoothingMode currentMode = SmoothingMode::Medium;

    inline float dBToGain(float dB)
    {
        return std::pow(10.0f, dB / 20.0f);
    }
};