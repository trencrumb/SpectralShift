/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <signalsmith-stretch/signalsmith-stretch.h>
#include "DSP/TiltEQ.h"
#include "DSP/TransientShaper.h"

//==============================================================================
/**
*/
class SpectralShiftAudioProcessor  : public juce::AudioProcessor,
                                      public juce::ValueTree::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SpectralShiftAudioProcessor();
    ~SpectralShiftAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    // Give DSP initial values
    void init();

    // Pass sample rate and buffer size to DSP 
    void prepare(double sampleRate, int samplesPerBlock);

    void setPreset(int index);

    // Called when user changes parameters
    void update();

    // Reset DSP parameters
    void reset() override;

    // Store Parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    bool isSpectralReady { false };
    void setCurrentEnvelope(const std::vector<float>& envelope);


private:

    bool isActive{ false };
    bool mustUpdateProcessing{ false };

    //juce::LinearSmoothedValue<float> driveNormal{ 0.0 };
    //juce::LinearSmoothedValue<float> outputVolume[2]{ 0.0 };
    //juce::LinearSmoothedValue<float> outputMix[2]{ 0.0 };

    signalsmith::stretch::SignalsmithStretch<float> stretch;
    float currentPitchSemitones { 0.0f };
    float currentFormantSemitones  { 0.0f };
    bool currentFormantPreservation { true };
    float currentTonalityHz { 0.0f };
    float currentFormantBaseHz { 0.0f };

    TiltEQ tiltEQ;
    float currentTiltCentreHz { 0.0f };
    float currentTiltGainDB { 0.0f };

    TransientShaper transientShaper;
    float currentAttackDB { 0.0f };
    float currentSustainDB { 0.0f };

    juce::AudioBuffer<float> stretchBuffer;
    std::vector<float*> inPtrs, outPtrs;

    struct Preset
    {
        juce::String name;
        std::map<juce::String, float> values; // paramID -> value
    };

    std::vector<Preset> presets;
    int currentPresetIndex = -1;






    // Called when user changes a parameter
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override
    {
        mustUpdateProcessing = true;
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralShiftAudioProcessor)
};