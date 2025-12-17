//
// Created by Kyle Ramsey on 12/12/2025.
// Adapted from https://github.com/jcurtis4207/Juce-Plugins/blob/master/Tilt-eq/Source/Tilt-eq.h
//

#pragma once
#include <juce_dsp/juce_dsp.h>

class TiltEQ
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        filterChain.prepare (spec);
        filterChain.reset();

        // Initialize smoothed value with 50ms ramp time
        smoothedCentreFreq.reset(sampleRate, 0.05);
        smoothedCentreFreq.setCurrentAndTargetValue(centreFreq);

        updateCoefficients();
    }

    void reset()
    {
        filterChain.reset();
    }

    void setCentreFrequency (float newFreq)
    {
        if (centreFreq != newFreq)
        {
            centreFreq = newFreq;
            smoothedCentreFreq.setTargetValue(newFreq);
            needsUpdate = true;
        }
    }

    void setGainDb (float newGainDb)
    {
        if (gainDb != newGainDb)
        {
            gainDb = newGainDb;
            needsUpdate = true;
        }
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // Update coefficients if needed (once per block is fine for tilt EQ)
        if (needsUpdate || smoothedCentreFreq.isSmoothing())
        {
            float targetFreq = smoothedCentreFreq.getNextValue();
            updateCoefficientsWithFreq(targetFreq);

            if (!smoothedCentreFreq.isSmoothing())
                needsUpdate = false;
        }

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        filterChain.process (context);
    }

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;
    using StereoFilter = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;

    juce::dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;

    double sampleRate = 44100.0;
    float centreFreq  = 1000.0f;
    float gainDb      = 0.0f;

    bool needsUpdate = true;

    juce::SmoothedValue<float> smoothedCentreFreq;

    void updateCoefficientsWithFreq(float freq)
    {
        const float q = 0.4f;

        auto lowGain  = juce::Decibels::decibelsToGain (-gainDb);
        auto highGain = juce::Decibels::decibelsToGain ( gainDb);

        *filterChain.get<0>().state =
            *Coeffs::makeLowShelf (sampleRate, freq, q, lowGain);

        *filterChain.get<1>().state =
            *Coeffs::makeHighShelf (sampleRate, freq, q, highGain);
    }

    void updateCoefficients()
    {
        updateCoefficientsWithFreq(centreFreq);
        needsUpdate = false;
    }
};
