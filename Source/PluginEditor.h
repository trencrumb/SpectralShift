/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "PluginProcessor.h"
#include "Component/XYPad.h"

//==============================================================================
/**
*/
class SpectralShiftAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit SpectralShiftAudioProcessorEditor (SpectralShiftAudioProcessor&);
    ~SpectralShiftAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // using Attachment to shorten the type
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

private:

    SpectralShiftAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> pitchSemitonesSlider, pitchCentsSlider,
        formantSemitonesSlider, formantCentsSlider,
        formantBaseHzSlider, tonalityHzSlider,
        transientAttackDBSlider, transientSustainDBSlider,
        tiltCentreHzSlider, tiltGainDbSlider;

    std::unique_ptr<juce::ToggleButton> formantCompensationToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> formantCompensationAttachment;

    std::unique_ptr<juce::Label> pitchSemitonesLabel, pitchCentsLabel,
        formantSemitonesLabel, formantCentsLabel,
        formantBaseHzLabel, tonalityHzLabel,
        transientAttackDBLabel, transientSustainDBLabel,
        tiltCentreHzLabel, tiltGainDbLabel;

    std::unique_ptr<Attachment> pitchSemitonesAttachment, pitchCentsAttachment,
        formantSemitonesAttachment, formantCentsAttachment,
        formantBaseHzAttachment, tonalityHzAttachment,
        transientAttackDBAttachment, transientSustainDBAttachment,
        tiltCentreHzAttachment, tiltGainDbAttachment;

    XYPad xyPad;

    // control whether all slider text boxes are editable (single central flag)
    static constexpr bool editableTextLabels = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralShiftAudioProcessorEditor)
};
