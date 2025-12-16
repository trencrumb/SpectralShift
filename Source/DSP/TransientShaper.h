//
// Created by Kyle Ramsey on 15/12/2025.
//

#pragma once
#include "EnvelopeFollower.h"
#include <algorithm>

class TransientShaper
{
public:
    void prepare(double sampleRate)
    {
        fastEnv.prepare(sampleRate);
        slowEnv.prepare(sampleRate);

        fastEnv.setAttackRelease(1.0f, 10.0f);
        slowEnv.setAttackRelease(10.0f, 100.0f);
    }

    void setParameters(float attack, float sustain)
    {
        attackAmount  = attack;
        sustainAmount = sustain;
    }

    float processSample(float x)
    {
        float fast = fastEnv.process(x);
        float slow = slowEnv.process(x);

        float transient = std::max(0.0f, fast - slow);

        float attackGain  = 1.0f + attackAmount  * transient;
        float sustainGain = 1.0f + sustainAmount * (1.0f - transient);

        return x * (attackGain + sustainGain);
    }

private:
    EnvelopeFollower fastEnv, slowEnv;
    float attackAmount  = 0.0f;
    float sustainAmount = 0.0f;
};
