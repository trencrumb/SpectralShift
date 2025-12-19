/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "PluginProcessor.h"
#include "Component/XYPad.h"
#include "Component/CustomLookAndFeel.h"

//==============================================================================
class SpectralShiftAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit SpectralShiftAudioProcessorEditor (SpectralShiftAudioProcessor&);
    ~SpectralShiftAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

private:
    SpectralShiftAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;

    // ========== Main XY Pad Section ==========
    XYPad xyPad;

    // Pitch semitones (X-axis of XY pad)
    std::unique_ptr<juce::Slider> pitchSemitonesSlider;
    std::unique_ptr<juce::Label> pitchSemitonesLabel;
    std::unique_ptr<juce::Label> pitchStaticLabel;
    std::unique_ptr<Attachment> pitchSemitonesAttachment;

    // Formant semitones (Y-axis of XY pad)
    std::unique_ptr<juce::Slider> formantSemitonesSlider;
    std::unique_ptr<juce::Label> formantSemitonesLabel;
    std::unique_ptr<juce::Label> formantStaticLabel;
    std::unique_ptr<Attachment> formantSemitonesAttachment;

    // ========== Cents Sliders (sides of XY pad) ==========
    std::unique_ptr<juce::Slider> pitchCentsSlider;
    std::unique_ptr<juce::Label> pitchCentsLabel;
    std::unique_ptr<Attachment> pitchCentsAttachment;

    std::unique_ptr<juce::Slider> formantCentsSlider;
    std::unique_ptr<juce::Label> formantCentsLabel;
    std::unique_ptr<Attachment> formantCentsAttachment;

    // ========== Tilt EQ Slider ==========
    std::unique_ptr<juce::Slider> tiltGainDbSlider;
    std::unique_ptr<juce::Label> tiltGainLabel;
    std::unique_ptr<juce::Label> tiltValueLabel;
    std::unique_ptr<Attachment> tiltGainDbAttachment;

    std::unique_ptr<juce::Slider> tiltCentreHzSlider;
    std::unique_ptr<juce::Label> tiltCentreLabel;
    std::unique_ptr<juce::Label> tiltCentreValueLabel;
    std::unique_ptr<Attachment> tiltCentreHzAttachment;

    std::unique_ptr<juce::ToggleButton> tiltCentreAutoToggle;
    std::unique_ptr<juce::Label> tiltCentreAutoLabel;
    std::unique_ptr<ButtonAttachment> tiltCentreAutoAttachment;

    // ========== Additional Controls ==========
    std::unique_ptr<juce::Slider> tonalityHzSlider;
    std::unique_ptr<juce::Label> tonalityHzLabel;
    std::unique_ptr<juce::Label> tonalityValueLabel;
    std::unique_ptr<Attachment> tonalityHzAttachment;

    std::unique_ptr<juce::ToggleButton> formantCompensationToggle;
    std::unique_ptr<juce::Label> formantCompLabel;
    std::unique_ptr<ButtonAttachment> formantCompensationAttachment;

    // ========== CPU Load Display ==========
    std::unique_ptr<juce::Label> cpuLoadLabel;

    // Helper method
    void updateBandSelection(int bandIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralShiftAudioProcessorEditor)
};