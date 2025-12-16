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
        if (needsUpdate)
            updateCoefficients();

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

    void updateCoefficients()
    {
        const float q = 0.4f;

        auto lowGain  = juce::Decibels::decibelsToGain (-gainDb);
        auto highGain = juce::Decibels::decibelsToGain ( gainDb);

        *filterChain.get<0>().state =
            *Coeffs::makeLowShelf (sampleRate, centreFreq, q, lowGain);

        *filterChain.get<1>().state =
            *Coeffs::makeHighShelf (sampleRate, centreFreq, q, highGain);

        needsUpdate = false;
    }
};
