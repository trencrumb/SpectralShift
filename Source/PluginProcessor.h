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
#include "DSP/SpectralCentroid.h"
#include "PresetManager.h"

#if PERFETTO
    #include <melatonin_perfetto/melatonin_perfetto.h>
#endif

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

    // Get current CPU load (0.0 to 1.0, where 1.0 = 100%)
    double getCpuLoad() const { return loadMeasurer.getLoadAsPercentage() / 100.0; }

    // Get preset manager for UI access
    PresetManager& getPresetManager() { return presetManager; }

    // Pass sample rate and buffer size to DSP
    void prepare(double sampleRate, int samplesPerBlock);

    // Called when user changes parameters
    void update();

    // Reset DSP parameters
    void reset() override;

    // Store Parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    bool isSpectralReady { false };
    static constexpr float semitonesRangeSt = 24.0f;
    // void setCurrentEnvelope(const std::vector<float>& envelope);


private:

    bool isActive{ false };
    bool mustUpdateProcessing{ false };

    signalsmith::stretch::SignalsmithStretch<float> stretch;
    float currentPitchSemitones { 0.0f };
    float currentFormantSemitones  { 0.0f };
    bool currentFormantPreservation { true };
    float currentTonalityHz { 0.0f };
    float currentFormantBaseHz { 0.0f };

    TiltEQ tiltEQ;
    float currentTiltGainDB { 0.0f };

    SpectralCentroid spectralCentroid;

    // CPU load measurement
    juce::AudioProcessLoadMeasurer loadMeasurer;

    // ===== Constants =====
    static constexpr float minTiltCentreHz = 200.0f;
    static constexpr float maxTiltCentreHz = 20000.0f;
    static constexpr float minFormantBaseHz = 20.0f;
    static constexpr float maxFormantBaseHz = 2000.0f;

#if PERFETTO
    MelatoninPerfetto perfettoSession;
#endif

    juce::AudioBuffer<float> stretchBuffer;
    std::vector<float> monoBuffer;
    std::vector<float*> inPtrs, outPtrs;

    // ===== Preset Management =====
    PresetManager presetManager;






    // ===== ProcessBlock Helper Methods =====
    /** Converts mono buffer from stereo input. */
    void createMonoSum(const juce::AudioBuffer<float>& buffer, int numSamples, int numChannels);

    /** Processes spectral shift using signalsmith stretch. */
    void processSpectralShift(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels);

    /** Calculates tilt EQ center frequency and applies tilt filter. */
    void calculateAndApplyTiltEQ(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels);

    // Called when user changes a parameter
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override
    {
        mustUpdateProcessing = true;
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralShiftAudioProcessor)
};

