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
class SpectralShiftAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit SpectralShiftAudioProcessorEditor (SpectralShiftAudioProcessor&);
    ~SpectralShiftAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

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

    // ========== Transient Shaper Section ==========
    // Band selector buttons
    std::unique_ptr<juce::TextButton> lowBandButton;
    std::unique_ptr<juce::TextButton> midBandButton;
    std::unique_ptr<juce::TextButton> highBandButton;
    int currentBand = 0; // 0=Low, 1=Mid, 2=High

    // Attack and Sustain sliders (vertical)
    std::unique_ptr<juce::Slider> transientAttackSlider;
    std::unique_ptr<juce::Slider> transientSustainSlider;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<juce::Label> sustainLabel;
    std::unique_ptr<juce::Label> attackValueLabel;
    std::unique_ptr<juce::Label> sustainValueLabel;

    // Attachments for each band
    std::unique_ptr<Attachment> lowAttackAttachment;
    std::unique_ptr<Attachment> lowSustainAttachment;
    std::unique_ptr<Attachment> midAttackAttachment;
    std::unique_ptr<Attachment> midSustainAttachment;
    std::unique_ptr<Attachment> highAttackAttachment;
    std::unique_ptr<Attachment> highSustainAttachment;

    // Smoothing mode selector
    std::unique_ptr<juce::ComboBox> smoothingComboBox;
    std::unique_ptr<juce::Label> smoothingLabel;
    std::unique_ptr<ComboBoxAttachment> smoothingAttachment;

    // ========== Crossover Frequency Controls ==========
    std::unique_ptr<juce::Slider> lowMidXoverSlider;
    std::unique_ptr<juce::Slider> midHighXoverSlider;
    std::unique_ptr<juce::Label> lowMidXoverLabel;
    std::unique_ptr<juce::Label> midHighXoverLabel;
    std::unique_ptr<Attachment> lowMidXoverAttachment;
    std::unique_ptr<Attachment> midHighXoverAttachment;

    // ========== Advanced Parameters Panel ==========
    std::unique_ptr<juce::TextButton> advancedToggleButton;
    bool showAdvancedPanel = false;

    // Advanced parameter controls
    std::unique_ptr<juce::Slider> tonalityHzSlider;
    std::unique_ptr<juce::Slider> formantBaseHzSlider;
    std::unique_ptr<juce::Slider> tiltCentreHzSlider;
    std::unique_ptr<juce::ToggleButton> formantCompensationToggle;

    std::unique_ptr<juce::Label> tonalityHzLabel;
    std::unique_ptr<juce::Label> formantBaseHzLabel;
    std::unique_ptr<juce::Label> tiltCentreHzLabel;
    std::unique_ptr<juce::Label> formantCompLabel;

    std::unique_ptr<Attachment> tonalityHzAttachment;
    std::unique_ptr<Attachment> formantBaseHzAttachment;
    std::unique_ptr<Attachment> tiltCentreHzAttachment;
    std::unique_ptr<ButtonAttachment> formantCompensationAttachment;

    // Helper methods
    void updateBandSelection(int bandIndex);
    void switchToBand(int bandIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralShiftAudioProcessorEditor)
};