/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PluginTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor&);
    ~PluginTemplateAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // using Attachment to shorten the type
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

private:

    // Slider Objects
    std::unique_ptr<juce::Slider> driveSlider, volSlider, mixSlider;

    // Slider Label Objects
    std::unique_ptr<juce::Label> driveLabel, volLabel, mixLabel;


    // Attachments
    std::unique_ptr<Attachment> driveAttachment, volAttachment, mixAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginTemplateAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessorEditor)
};