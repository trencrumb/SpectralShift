//
// Created by Kyle Ramsey on 15/12/2025.
//

#pragma once
#include <cmath>

class EnvelopeFollower
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
    }

    void setAttackRelease(float attackMs, float releaseMs)
    {
        attackCoeff  = std::exp(-1.0f / (0.001f * attackMs  * sr));
        releaseCoeff = std::exp(-1.0f / (0.001f * releaseMs * sr));
    }

    float process(float x)
    {
        x = std::abs(x);

        if (x > env)
            env = attackCoeff * env + (1.0f - attackCoeff) * x;
        else
            env = releaseCoeff * env + (1.0f - releaseCoeff) * x;

        return env;
    }

private:
    double sr = 44100.0;
    double env = 0.0f;
    double attackCoeff = 0.99f;
    double releaseCoeff = 0.999f;
};