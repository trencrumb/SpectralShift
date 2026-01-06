#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectralShiftAudioProcessorEditor::SpectralShiftAudioProcessorEditor (SpectralShiftAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    // ========== XY Pad Setup ==========
    addAndMakeVisible(xyPad);

    // Customize XYPad colors (optional - defaults are already set)
    xyPad.setThumbColours(
        CustomLookAndFeel::Colors::pitchPositive,
        CustomLookAndFeel::Colors::pitchNegative,
        CustomLookAndFeel::Colors::formantPositive,
        CustomLookAndFeel::Colors::formantNegative
    );
    xyPad.setBackgroundColour(CustomLookAndFeel::Colors::backgroundDark);

    float semitonesRange = SpectralShiftAudioProcessor::semitonesRangeSt;

    // Pitch semitones (X-axis) - hidden slider for XY pad
    pitchSemitonesSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    pitchSemitonesSlider->setRange(-semitonesRange, semitonesRange, 0.01);
    addAndMakeVisible(*pitchSemitonesSlider);
    pitchSemitonesAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "PITCH_SEMITONES", *pitchSemitonesSlider);

    // Pitch label
    pitchStaticLabel = std::make_unique<juce::Label>("", "PITCH");
    pitchStaticLabel->setJustificationType(juce::Justification::centred);
    pitchStaticLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::pitchPositive);
    pitchStaticLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*pitchStaticLabel);

    pitchSemitonesLabel = std::make_unique<juce::Label>();
    pitchSemitonesLabel->setText(juce::String(audioProcessor.apvts.getRawParameterValue("PITCH_SEMITONES")->load(), 1) + " st", juce::dontSendNotification);
    pitchSemitonesLabel->setJustificationType(juce::Justification::centred);
    pitchSemitonesLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    pitchSemitonesLabel->setColour(juce::Label::backgroundColourId, CustomLookAndFeel::Colors::transparent);
    pitchSemitonesLabel->setColour(juce::Label::outlineColourId, CustomLookAndFeel::Colors::transparent);
    pitchSemitonesLabel->setFont(juce::FontOptions(14.0f, juce::Font::bold));
    pitchSemitonesLabel->setEditable(true);
    pitchSemitonesLabel->onTextChange = [this, semitonesRange]() {
        float value = pitchSemitonesLabel->getText().retainCharacters("-0123456789.").getFloatValue();
        value = juce::jlimit(-semitonesRange, semitonesRange, value);
        if (auto* param = audioProcessor.apvts.getParameter("PITCH_SEMITONES"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(value));
    };
    addAndMakeVisible(*pitchSemitonesLabel);

    pitchSemitonesSlider->onValueChange = [this]() {
        pitchSemitonesLabel->setText(juce::String(pitchSemitonesSlider->getValue(), 1) + " st", juce::dontSendNotification);
    };

    // Formant semitones (Y-axis) - hidden slider for XY pad
    formantSemitonesSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    formantSemitonesSlider->setRange(-semitonesRange, semitonesRange, 0.01);
    addAndMakeVisible(*formantSemitonesSlider);
    formantSemitonesAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "FORMANT_SEMITONES", *formantSemitonesSlider);

    // Formant label
    formantStaticLabel = std::make_unique<juce::Label>("", "FORMANT");
    formantStaticLabel->setJustificationType(juce::Justification::centred);
    formantStaticLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::formantPositive);
    formantStaticLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*formantStaticLabel);

    formantSemitonesLabel = std::make_unique<juce::Label>();
    formantSemitonesLabel->setText(juce::String(audioProcessor.apvts.getRawParameterValue("FORMANT_SEMITONES")->load(), 1) + " st", juce::dontSendNotification);
    formantSemitonesLabel->setJustificationType(juce::Justification::centred);
    formantSemitonesLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    formantSemitonesLabel->setColour(juce::Label::backgroundColourId, CustomLookAndFeel::Colors::transparent);
    formantSemitonesLabel->setColour(juce::Label::outlineColourId, CustomLookAndFeel::Colors::transparent);
    formantSemitonesLabel->setFont(juce::FontOptions(14.0f, juce::Font::bold));
    formantSemitonesLabel->setEditable(true);
    formantSemitonesLabel->onTextChange = [this, semitonesRange]() {
        float value = formantSemitonesLabel->getText().retainCharacters("-0123456789.").getFloatValue();
        value = juce::jlimit(-semitonesRange, semitonesRange, value);
        if (auto* param = audioProcessor.apvts.getParameter("FORMANT_SEMITONES"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(value));
    };
    addAndMakeVisible(*formantSemitonesLabel);

    formantSemitonesSlider->onValueChange = [this]() {
        formantSemitonesLabel->setText(juce::String(formantSemitonesSlider->getValue(), 1) + " st", juce::dontSendNotification);
    };

    // Register sliders with XY pad
    xyPad.registerSlider(pitchSemitonesSlider.get(), XYPad::Axis::X);
    xyPad.registerSlider(formantSemitonesSlider.get(), XYPad::Axis::Y);

    // ========== Cents Sliders ==========
    // Pitch cents (left side)
    pitchCentsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
    pitchCentsSlider->setName("CentsSlider");
    pitchCentsSlider->setRange(-200.0, 200.0, 1.0);
    pitchCentsSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::pitchPositive);
    addAndMakeVisible(*pitchCentsSlider);
    pitchCentsAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "PITCH_CENTS", *pitchCentsSlider);

    pitchCentsLabel = std::make_unique<juce::Label>("", "0c");
    pitchCentsLabel->setJustificationType(juce::Justification::centred);
    pitchCentsLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    pitchCentsLabel->setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(*pitchCentsLabel);

    pitchCentsSlider->onValueChange = [this]() {
        pitchCentsLabel->setText(juce::String(static_cast<int>(pitchCentsSlider->getValue())) + "c", juce::dontSendNotification);
    };

    // Formant cents (right side)
    formantCentsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
    formantCentsSlider->setName("CentsSlider");
    formantCentsSlider->setRange(-200.0, 200.0, 1.0);
    formantCentsSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::formantPositive);
    addAndMakeVisible(*formantCentsSlider);
    formantCentsAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "FORMANT_CENTS", *formantCentsSlider);

    formantCentsLabel = std::make_unique<juce::Label>("", "0c");
    formantCentsLabel->setJustificationType(juce::Justification::centred);
    formantCentsLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    formantCentsLabel->setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(*formantCentsLabel);

    formantCentsSlider->onValueChange = [this]() {
        formantCentsLabel->setText(juce::String(static_cast<int>(formantCentsSlider->getValue())) + "c", juce::dontSendNotification);
    };

    // ========== Tilt EQ Slider ==========
    tiltGainDbSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    tiltGainDbSlider->setName("TiltSlider");
    tiltGainDbSlider->setRange(-6.0, 6.0, 0.01);
    tiltGainDbSlider->setSkewFactorFromMidPoint(0.0);
    tiltGainDbSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::accent);
    addAndMakeVisible(*tiltGainDbSlider);
    tiltGainDbAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TILT_GAIN_DB", *tiltGainDbSlider);

    tiltGainLabel = std::make_unique<juce::Label>("", "TILT");
    tiltGainLabel->setJustificationType(juce::Justification::centredLeft);
    tiltGainLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::tilt);
    tiltGainLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*tiltGainLabel);

    tiltValueLabel = std::make_unique<juce::Label>("", "0.0 dB");
    tiltValueLabel->setJustificationType(juce::Justification::centredRight);
    tiltValueLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    tiltValueLabel->setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(*tiltValueLabel);

    tiltGainDbSlider->onValueChange = [this]() {
        tiltValueLabel->setText(juce::String(tiltGainDbSlider->getValue(), 1) + " dB", juce::dontSendNotification);
    };

    tiltCentreHzSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    tiltCentreHzSlider->setName("TonalitySlider");
    tiltCentreHzSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::tilt);
    addAndMakeVisible(*tiltCentreHzSlider);
    tiltCentreHzAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TILT_CENTRE_HZ", *tiltCentreHzSlider);  // Range & skew from parameter

    // Labels (similar to other sliders)
    tiltCentreLabel = std::make_unique<juce::Label>("", "TILT CENTRE");
    tiltCentreLabel->setJustificationType(juce::Justification::centredLeft);
    tiltCentreLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::tilt);
    tiltCentreLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*tiltCentreLabel);

    tiltCentreValueLabel = std::make_unique<juce::Label>("", "1000 Hz");
    tiltCentreValueLabel->setJustificationType(juce::Justification::centredRight);
    tiltCentreValueLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    tiltCentreValueLabel->setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(*tiltCentreValueLabel);

    tiltCentreHzSlider->onValueChange = [this]() {
        tiltCentreValueLabel->setText(juce::String(static_cast<int>(tiltCentreHzSlider->getValue())) + " Hz", juce::dontSendNotification);
    };

    // Toggle for auto mode
    tiltCentreAutoLabel = std::make_unique<juce::Label>("", "AUTO TILT CENTRE");
    tiltCentreAutoLabel->setJustificationType(juce::Justification::centredLeft);
    tiltCentreAutoLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::tilt);
    tiltCentreAutoLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*tiltCentreAutoLabel);

    tiltCentreAutoToggle = std::make_unique<juce::ToggleButton>("");
    tiltCentreAutoToggle->setName("Tilt");
    addAndMakeVisible(*tiltCentreAutoToggle);
    tiltCentreAutoAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "TILT_CENTRE_AUTO", *tiltCentreAutoToggle);

    tiltCentreAutoToggle->onClick = [this]() {
        bool isAuto = tiltCentreAutoToggle->getToggleState();

        // Disable manual input when auto
        tiltCentreHzSlider->setEnabled(!isAuto);

        // Change color to lighter/dimmed version when auto
        if (isAuto) {
            tiltCentreHzSlider->setColour(juce::Slider::trackColourId,
                CustomLookAndFeel::Colors::tilt.withAlpha(0.5f));
        } else {
            tiltCentreHzSlider->setColour(juce::Slider::trackColourId,
                CustomLookAndFeel::Colors::tilt);
        }
    };

    tiltCentreAutoToggle->onClick();




    // ========== Tonality Limit Slider ==========
    tonalityHzSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    tonalityHzSlider->setName("TonalitySlider");
    tonalityHzSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::tonality);
    addAndMakeVisible(*tonalityHzSlider);
    tonalityHzAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TONALITY_HZ", *tonalityHzSlider);  // Range & skew from parameter

    tonalityHzLabel = std::make_unique<juce::Label>("", "TONALITY LIMIT");
    tonalityHzLabel->setJustificationType(juce::Justification::centredLeft);
    tonalityHzLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::tonality);
    tonalityHzLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*tonalityHzLabel);

    tonalityValueLabel = std::make_unique<juce::Label>("", juce::String(static_cast<int>(tonalityHzSlider->getValue())) + " Hz");
    tonalityValueLabel->setJustificationType(juce::Justification::centredRight);
    tonalityValueLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    tonalityValueLabel->setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(*tonalityValueLabel);

    tonalityHzSlider->onValueChange = [this]() {
        tonalityValueLabel->setText(juce::String(static_cast<int>(tonalityHzSlider->getValue())) + " Hz", juce::dontSendNotification);
    };

    // ========== Formant Compensation Toggle ==========
    // Label for text
    formantCompLabel = std::make_unique<juce::Label>("", "FORMANT COMPENSATION");
    formantCompLabel->setJustificationType(juce::Justification::centredLeft);
    formantCompLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::formantCompensation);
    formantCompLabel->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(*formantCompLabel);

    // Toggle button (no text, just the box)
    formantCompensationToggle = std::make_unique<juce::ToggleButton>("");
    formantCompensationToggle->setName("Formant");
    addAndMakeVisible(*formantCompensationToggle);
    formantCompensationAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "FORMANT_COMPENSATION", *formantCompensationToggle);

    // ========== CPU Load Display ==========
    cpuLoadLabel = std::make_unique<juce::Label>("", "CPU: 0%");
    cpuLoadLabel->setJustificationType(juce::Justification::centredRight);
    cpuLoadLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    cpuLoadLabel->setFont(juce::FontOptions(9.0f));
    addAndMakeVisible(*cpuLoadLabel);

    // Start timer to update CPU display (30 Hz refresh rate)
    startTimerHz(30);

    setSize(400, 650);
}

SpectralShiftAudioProcessorEditor::~SpectralShiftAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    xyPad.deregisterSlider(pitchSemitonesSlider.get());
    xyPad.deregisterSlider(formantSemitonesSlider.get());
}

//==============================================================================
void SpectralShiftAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(CustomLookAndFeel::Colors::background);

    // Add subtle vignette
    auto bounds = getLocalBounds();
    juce::ColourGradient vignette(
        CustomLookAndFeel::Colors::background, static_cast<float>(bounds.getCentreX()), static_cast<float>(bounds.getCentreY()),
        CustomLookAndFeel::Colors::backgroundDark, static_cast<float>(bounds.getX()), static_cast<float>(bounds.getY()),
        true
    );
    g.setGradientFill(vignette);
    g.fillAll();

    // Draw section dividers
    g.setColour(CustomLookAndFeel::Colors::backgroundLight);
    // Line below XY pad section
    //g.drawLine(20, 400, getWidth() - 20, 400, 1.0f);
    // Line below tilt section
    g.drawLine(20, 460, static_cast<float>(getWidth() - 20), 460, 1.0f);
}

void SpectralShiftAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    constexpr int padding = 10;

    // ========== XY Pad Section (top) ==========
    auto xyPadArea = area.removeFromTop(340);

    // Labels above XY pad - side by side
    auto labelsArea = xyPadArea.removeFromTop(40);
    int labelWidth = 100;
    // int labelHeight = 25;

    int totalLabelsWidth = labelWidth * 2 + 20;  // Two labels with 20 px gap
    labelsArea = labelsArea.withSizeKeepingCentre(totalLabelsWidth, 40);

    auto pitchLabelArea = labelsArea.removeFromLeft(labelWidth);
    pitchStaticLabel->setBounds(pitchLabelArea.removeFromTop(15));
    pitchSemitonesLabel->setBounds(pitchLabelArea);

    labelsArea.removeFromLeft(20);  // Gap between labels

    auto formantLabelArea = labelsArea.removeFromLeft(labelWidth);
    formantStaticLabel->setBounds(formantLabelArea.removeFromTop(15));
    formantSemitonesLabel->setBounds(formantLabelArea);

    // Cents sliders on sides
    auto leftSliderArea = xyPadArea.removeFromLeft(40);
    auto rightSliderArea = xyPadArea.removeFromRight(40);

    // Pitch cents (left)
    pitchCentsLabel->setBounds(leftSliderArea.removeFromBottom(20));
    pitchCentsSlider->setBounds(leftSliderArea.reduced(10, 0));

    // Formant cents (right)
    formantCentsLabel->setBounds(rightSliderArea.removeFromBottom(20));
    formantCentsSlider->setBounds(rightSliderArea.reduced(10, 0));

    // XY Pad in center (make it square)
    int padSize = juce::jmin(xyPadArea.getWidth(), xyPadArea.getHeight());
    auto padArea = xyPadArea.withSizeKeepingCentre(padSize, padSize);
    xyPad.setBounds(padArea);

    area.removeFromTop(padding);

    // ========== Formant Compensation Toggle ==========
    auto toggleArea = area.removeFromTop(25);
    formantCompLabel->setBounds(toggleArea.removeFromLeft(toggleArea.getWidth() - 30));
    formantCompensationToggle->setBounds(toggleArea);

    area.removeFromTop(padding);

    // ========== Tonality Limit Slider ==========
    auto tonalityArea = area.removeFromTop(50);
    auto tonalityLabelArea = tonalityArea.removeFromTop(20);
    tonalityHzLabel->setBounds(tonalityLabelArea.removeFromLeft(120));
    tonalityValueLabel->setBounds(tonalityLabelArea.removeFromRight(80));
    tonalityHzSlider->setBounds(tonalityArea.reduced(10, 0));

    area.removeFromTop(padding);

    // ========== Tilt Slider Section ==========
    auto tiltArea = area.removeFromTop(50);
    auto tiltLabelArea = tiltArea.removeFromTop(20);
    tiltGainLabel->setBounds(tiltLabelArea.removeFromLeft(60));
    tiltValueLabel->setBounds(tiltLabelArea.removeFromRight(60));
    tiltGainDbSlider->setBounds(tiltArea.reduced(10, 0));

    area.removeFromTop(padding);

    // Tilt Centre Hz Slider
    auto tiltCentreArea = area.removeFromTop(50);
    auto tiltCentreLabelArea = tiltCentreArea.removeFromTop(20);
    tiltCentreLabel->setBounds(tiltCentreLabelArea.removeFromLeft(100));
    tiltCentreValueLabel->setBounds(tiltCentreLabelArea.removeFromRight(80));
    tiltCentreHzSlider->setBounds(tiltCentreArea.reduced(10, 0));

    area.removeFromTop(padding);

    // Auto toggle
    auto autoToggleArea = area.removeFromTop(25);
    tiltCentreAutoLabel->setBounds(autoToggleArea.removeFromLeft(autoToggleArea.getWidth() - 30));
    tiltCentreAutoToggle->setBounds(autoToggleArea);

    // CPU Load Display (bottom-right corner)
    cpuLoadLabel->setBounds(getWidth() - 80, getHeight() - 20, 70, 15);
}

void SpectralShiftAudioProcessorEditor::timerCallback()
{
    // Update CPU load display
    double cpuLoad = audioProcessor.getCpuLoad();
    int cpuPercent = static_cast<int>(cpuLoad * 100.0);
    cpuLoadLabel->setText("CPU: " + juce::String(cpuPercent) + "%", juce::dontSendNotification);

    // Change color if CPU is high
    if (cpuLoad > 0.8)  // Over 80%
        cpuLoadLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::error);
    else if (cpuLoad > 0.5)  // Over 50%
        cpuLoadLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::accent);
    else
        cpuLoadLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
}

