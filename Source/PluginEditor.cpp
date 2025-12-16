#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectralShiftAudioProcessorEditor::SpectralShiftAudioProcessorEditor (SpectralShiftAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    // ========== XY Pad Setup ==========
    addAndMakeVisible(xyPad);

    // Pitch semitones (X-axis) - hidden slider for XY pad
    pitchSemitonesSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    pitchSemitonesSlider->setRange(-12.0, 12.0, 0.01);
    addAndMakeVisible(*pitchSemitonesSlider);
    pitchSemitonesAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "PITCH_SEMITONES", *pitchSemitonesSlider);

    // Pitch label
    pitchStaticLabel = std::make_unique<juce::Label>("", "PITCH");
    pitchStaticLabel->setJustificationType(juce::Justification::centred);
    pitchStaticLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::primary);
    pitchStaticLabel->setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(*pitchStaticLabel);

    pitchSemitonesLabel = std::make_unique<juce::Label>();
    pitchSemitonesLabel->setText(juce::String(audioProcessor.apvts.getRawParameterValue("PITCH_SEMITONES")->load(), 1) + " st", juce::dontSendNotification);
    pitchSemitonesLabel->setJustificationType(juce::Justification::centred);
    pitchSemitonesLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    pitchSemitonesLabel->setColour(juce::Label::backgroundColourId, CustomLookAndFeel::Colors::backgroundDark);
    pitchSemitonesLabel->setColour(juce::Label::outlineColourId, CustomLookAndFeel::Colors::primary);
    pitchSemitonesLabel->setFont(juce::Font(14.0f, juce::Font::bold));
    pitchSemitonesLabel->setEditable(true);
    pitchSemitonesLabel->onTextChange = [this]() {
        float value = pitchSemitonesLabel->getText().retainCharacters("-0123456789.").getFloatValue();
        value = juce::jlimit(-12.0f, 12.0f, value);
        if (auto* param = audioProcessor.apvts.getParameter("PITCH_SEMITONES"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(value));
    };
    addAndMakeVisible(*pitchSemitonesLabel);

    pitchSemitonesSlider->onValueChange = [this]() {
        pitchSemitonesLabel->setText(juce::String(pitchSemitonesSlider->getValue(), 1) + " st", juce::dontSendNotification);
    };

    // Formant semitones (Y-axis) - hidden slider for XY pad
    formantSemitonesSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    formantSemitonesSlider->setRange(-12.0, 12.0, 0.01);
    addAndMakeVisible(*formantSemitonesSlider);
    formantSemitonesAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "FORMANT_SEMITONES", *formantSemitonesSlider);

    // Formant label
    formantStaticLabel = std::make_unique<juce::Label>("", "FORMANT");
    formantStaticLabel->setJustificationType(juce::Justification::centred);
    formantStaticLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::secondary);
    formantStaticLabel->setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(*formantStaticLabel);

    formantSemitonesLabel = std::make_unique<juce::Label>();
    formantSemitonesLabel->setText(juce::String(audioProcessor.apvts.getRawParameterValue("FORMANT_SEMITONES")->load(), 1) + " st", juce::dontSendNotification);
    formantSemitonesLabel->setJustificationType(juce::Justification::centred);
    formantSemitonesLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    formantSemitonesLabel->setColour(juce::Label::backgroundColourId, CustomLookAndFeel::Colors::backgroundDark);
    formantSemitonesLabel->setColour(juce::Label::outlineColourId, CustomLookAndFeel::Colors::secondary);
    formantSemitonesLabel->setFont(juce::Font(14.0f, juce::Font::bold));
    formantSemitonesLabel->setEditable(true);
    formantSemitonesLabel->onTextChange = [this]() {
        float value = formantSemitonesLabel->getText().retainCharacters("-0123456789.").getFloatValue();
        value = juce::jlimit(-12.0f, 12.0f, value);
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
    pitchCentsSlider->setRange(-200.0, 200.0, 1.0);
    pitchCentsSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::primary);
    addAndMakeVisible(*pitchCentsSlider);
    pitchCentsAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "PITCH_CENTS", *pitchCentsSlider);

    pitchCentsLabel = std::make_unique<juce::Label>("", "0c");
    pitchCentsLabel->setJustificationType(juce::Justification::centred);
    pitchCentsLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::primary);
    pitchCentsLabel->setFont(juce::Font(10.0f));
    addAndMakeVisible(*pitchCentsLabel);

    pitchCentsSlider->onValueChange = [this]() {
        pitchCentsLabel->setText(juce::String((int)pitchCentsSlider->getValue()) + "c", juce::dontSendNotification);
    };

    // Formant cents (right side)
    formantCentsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
    formantCentsSlider->setRange(-200.0, 200.0, 1.0);
    formantCentsSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::secondary);
    addAndMakeVisible(*formantCentsSlider);
    formantCentsAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "FORMANT_CENTS", *formantCentsSlider);

    formantCentsLabel = std::make_unique<juce::Label>("", "0c");
    formantCentsLabel->setJustificationType(juce::Justification::centred);
    formantCentsLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::secondary);
    formantCentsLabel->setFont(juce::Font(10.0f));
    addAndMakeVisible(*formantCentsLabel);

    formantCentsSlider->onValueChange = [this]() {
        formantCentsLabel->setText(juce::String((int)formantCentsSlider->getValue()) + "c", juce::dontSendNotification);
    };

    // ========== Tilt EQ Slider ==========
    tiltGainDbSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    tiltGainDbSlider->setRange(-6.0, 6.0, 0.01);
    tiltGainDbSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::accent);
    addAndMakeVisible(*tiltGainDbSlider);
    tiltGainDbAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TILT_GAIN_DB", *tiltGainDbSlider);

    tiltGainLabel = std::make_unique<juce::Label>("", "TILT");
    tiltGainLabel->setJustificationType(juce::Justification::centredLeft);
    tiltGainLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::accent);
    tiltGainLabel->setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(*tiltGainLabel);

    tiltValueLabel = std::make_unique<juce::Label>("", "0.0 dB");
    tiltValueLabel->setJustificationType(juce::Justification::centredRight);
    tiltValueLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    tiltValueLabel->setFont(juce::Font(10.0f));
    addAndMakeVisible(*tiltValueLabel);

    tiltGainDbSlider->onValueChange = [this]() {
        tiltValueLabel->setText(juce::String(tiltGainDbSlider->getValue(), 1) + " dB", juce::dontSendNotification);
    };

    // ========== Transient Shaper - Band Selector ==========
    lowBandButton = std::make_unique<juce::TextButton>("LOW");
    lowBandButton->setClickingTogglesState(true);
    lowBandButton->setRadioGroupId(1001);
    lowBandButton->setToggleState(true, juce::dontSendNotification);
    lowBandButton->setConnectedEdges(juce::Button::ConnectedOnRight);
    lowBandButton->onClick = [this]() { switchToBand(0); };
    addAndMakeVisible(*lowBandButton);

    midBandButton = std::make_unique<juce::TextButton>("MID");
    midBandButton->setClickingTogglesState(true);
    midBandButton->setRadioGroupId(1001);
    midBandButton->setConnectedEdges(juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    midBandButton->onClick = [this]() { switchToBand(1); };
    addAndMakeVisible(*midBandButton);

    highBandButton = std::make_unique<juce::TextButton>("HIGH");
    highBandButton->setClickingTogglesState(true);
    highBandButton->setRadioGroupId(1001);
    highBandButton->setConnectedEdges(juce::Button::ConnectedOnLeft);
    highBandButton->onClick = [this]() { switchToBand(2); };
    addAndMakeVisible(*highBandButton);

    // ========== Transient Attack/Sustain Sliders ==========
    transientAttackSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
    transientAttackSlider->setRange(-15.0, 15.0, 0.01);
    transientAttackSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::lowBand);
    addAndMakeVisible(*transientAttackSlider);

    transientSustainSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
    transientSustainSlider->setRange(-15.0, 15.0, 0.01);
    transientSustainSlider->setColour(juce::Slider::trackColourId, CustomLookAndFeel::Colors::lowBand);
    addAndMakeVisible(*transientSustainSlider);

    attackLabel = std::make_unique<juce::Label>("", "ATTACK");
    attackLabel->setJustificationType(juce::Justification::centred);
    attackLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    attackLabel->setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(*attackLabel);

    sustainLabel = std::make_unique<juce::Label>("", "SUSTAIN");
    sustainLabel->setJustificationType(juce::Justification::centred);
    sustainLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    sustainLabel->setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(*sustainLabel);

    attackValueLabel = std::make_unique<juce::Label>("", "0.0 dB");
    attackValueLabel->setJustificationType(juce::Justification::centred);
    attackValueLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::lowBand);
    attackValueLabel->setFont(juce::Font(10.0f));
    addAndMakeVisible(*attackValueLabel);

    sustainValueLabel = std::make_unique<juce::Label>("", "0.0 dB");
    sustainValueLabel->setJustificationType(juce::Justification::centred);
    sustainValueLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::lowBand);
    sustainValueLabel->setFont(juce::Font(10.0f));
    addAndMakeVisible(*sustainValueLabel);

    // Start with low band selected (only create attachments for low band initially)
    lowAttackAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "LOW_TRANS_ATTACK_DB", *transientAttackSlider);
    lowSustainAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "LOW_TRANS_SUSTAIN_DB", *transientSustainSlider);

    transientAttackSlider->onValueChange = [this]() {
        attackValueLabel->setText(juce::String(transientAttackSlider->getValue(), 1) + " dB", juce::dontSendNotification);
    };

    transientSustainSlider->onValueChange = [this]() {
        sustainValueLabel->setText(juce::String(transientSustainSlider->getValue(), 1) + " dB", juce::dontSendNotification);
    };

    // ========== Crossover Frequency Controls (in Advanced Panel) ==========
    lowMidXoverSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    lowMidXoverSlider->setRange(80.0, 1000.0, 1.0);
    lowMidXoverSlider->setSkewFactor(0.3);
    lowMidXoverSlider->setColour(juce::Slider::rotarySliderFillColourId, CustomLookAndFeel::Colors::accent);
    lowMidXoverSlider->setVisible(false);
    addAndMakeVisible(*lowMidXoverSlider);
    lowMidXoverAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TRANS_LOWMID_XOVER_HZ", *lowMidXoverSlider);

    lowMidXoverLabel = std::make_unique<juce::Label>("", "Low/Mid: 250 Hz");
    lowMidXoverLabel->setJustificationType(juce::Justification::centred);
    lowMidXoverLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    lowMidXoverLabel->setFont(juce::Font(10.0f));
    lowMidXoverLabel->setVisible(false);
    addAndMakeVisible(*lowMidXoverLabel);

    lowMidXoverSlider->onValueChange = [this]() {
        lowMidXoverLabel->setText("Low/Mid: " + juce::String((int)lowMidXoverSlider->getValue()) + " Hz", juce::dontSendNotification);
    };

    midHighXoverSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    midHighXoverSlider->setRange(1000.0, 10000.0, 1.0);
    midHighXoverSlider->setSkewFactor(0.3);
    midHighXoverSlider->setColour(juce::Slider::rotarySliderFillColourId, CustomLookAndFeel::Colors::accent);
    midHighXoverSlider->setVisible(false);
    addAndMakeVisible(*midHighXoverSlider);
    midHighXoverAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TRANS_MIDHIGH_XOVER_HZ", *midHighXoverSlider);

    midHighXoverLabel = std::make_unique<juce::Label>("", "Mid/High: 4000 Hz");
    midHighXoverLabel->setJustificationType(juce::Justification::centred);
    midHighXoverLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    midHighXoverLabel->setFont(juce::Font(10.0f));
    midHighXoverLabel->setVisible(false);
    addAndMakeVisible(*midHighXoverLabel);

    midHighXoverSlider->onValueChange = [this]() {
        midHighXoverLabel->setText("Mid/High: " + juce::String((int)midHighXoverSlider->getValue()) + " Hz", juce::dontSendNotification);
    };

    // ========== Smoothing Mode Selector ==========
    smoothingLabel = std::make_unique<juce::Label>("", "SMOOTHING");
    smoothingLabel->setJustificationType(juce::Justification::centredLeft);
    smoothingLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::textDim);
    smoothingLabel->setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(*smoothingLabel);

    smoothingComboBox = std::make_unique<juce::ComboBox>();
    smoothingComboBox->addItem("Sharp", 1);
    smoothingComboBox->addItem("Medium", 2);
    smoothingComboBox->addItem("Smooth", 3);
    addAndMakeVisible(*smoothingComboBox);
    smoothingAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "TRANS_SMOOTHING", *smoothingComboBox);

    // ========== Advanced Panel Toggle ==========
    advancedToggleButton = std::make_unique<juce::TextButton>("Advanced Parameters");
    advancedToggleButton->setClickingTogglesState(true);
    advancedToggleButton->onClick = [this]() {
        showAdvancedPanel = advancedToggleButton->getToggleState();
        resized();
    };
    addAndMakeVisible(*advancedToggleButton);

    // ========== Advanced Parameters (hidden by default) ==========
    tonalityHzSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    tonalityHzSlider->setRange(200.0, 20000.0, 1.0);
    tonalityHzSlider->setTextValueSuffix(" Hz");
    tonalityHzSlider->setColour(juce::Slider::rotarySliderFillColourId, CustomLookAndFeel::Colors::primary);
    tonalityHzSlider->setVisible(false);
    addAndMakeVisible(*tonalityHzSlider);
    tonalityHzAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TONALITY_HZ", *tonalityHzSlider);

    tonalityHzLabel = std::make_unique<juce::Label>("", "Tonality Hz");
    tonalityHzLabel->setJustificationType(juce::Justification::centred);
    tonalityHzLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    tonalityHzLabel->setFont(juce::Font(10.0f));
    tonalityHzLabel->setVisible(false);
    addAndMakeVisible(*tonalityHzLabel);

    formantBaseHzSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    formantBaseHzSlider->setRange(0.0, 500.0, 1.0);
    formantBaseHzSlider->setTextValueSuffix(" Hz");
    formantBaseHzSlider->setColour(juce::Slider::rotarySliderFillColourId, CustomLookAndFeel::Colors::secondary);
    formantBaseHzSlider->setVisible(false);
    addAndMakeVisible(*formantBaseHzSlider);
    formantBaseHzAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "FORMANT_BASE_HZ", *formantBaseHzSlider);

    formantBaseHzLabel = std::make_unique<juce::Label>("", "Formant Base Hz");
    formantBaseHzLabel->setJustificationType(juce::Justification::centred);
    formantBaseHzLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    formantBaseHzLabel->setFont(juce::Font(10.0f));
    formantBaseHzLabel->setVisible(false);
    addAndMakeVisible(*formantBaseHzLabel);

    tiltCentreHzSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    tiltCentreHzSlider->setRange(20.0, 20000.0, 1.0);
    tiltCentreHzSlider->setTextValueSuffix(" Hz");
    tiltCentreHzSlider->setColour(juce::Slider::rotarySliderFillColourId, CustomLookAndFeel::Colors::accent);
    tiltCentreHzSlider->setVisible(false);
    addAndMakeVisible(*tiltCentreHzSlider);
    tiltCentreHzAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "TILT_CENTRE_HZ", *tiltCentreHzSlider);

    tiltCentreHzLabel = std::make_unique<juce::Label>("", "Tilt Centre Hz");
    tiltCentreHzLabel->setJustificationType(juce::Justification::centred);
    tiltCentreHzLabel->setColour(juce::Label::textColourId, CustomLookAndFeel::Colors::text);
    tiltCentreHzLabel->setFont(juce::Font(10.0f));
    tiltCentreHzLabel->setVisible(false);
    addAndMakeVisible(*tiltCentreHzLabel);

    formantCompensationToggle = std::make_unique<juce::ToggleButton>("Formant Compensation");
    formantCompensationToggle->setVisible(false);
    addAndMakeVisible(*formantCompensationToggle);
    formantCompensationAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "FORMANT_COMPENSATION", *formantCompensationToggle);

    setSize(400, 700);
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
        CustomLookAndFeel::Colors::background, bounds.getCentreX(), bounds.getCentreY(),
        CustomLookAndFeel::Colors::backgroundDark, bounds.getX(), bounds.getY(),
        true
    );
    g.setGradientFill(vignette);
    g.fillAll();

    // Draw section dividers
    g.setColour(CustomLookAndFeel::Colors::backgroundLight);
    // Line below XY pad section
    g.drawLine(20, 400, getWidth() - 20, 400, 1.0f);
    // Line below tilt section
    g.drawLine(20, 460, getWidth() - 20, 460, 1.0f);
}

void SpectralShiftAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    const int padding = 10;

    // ========== XY Pad Section (top) ==========
    auto xyPadArea = area.removeFromTop(340);

    // Labels below XY pad - side by side
    auto labelsArea = xyPadArea.removeFromBottom(40);
    int labelWidth = 100;
    int labelHeight = 25;

    int totalLabelsWidth = labelWidth * 2 + 20;  // Two labels with 20px gap
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

    // ========== Tilt Slider Section ==========
    auto tiltArea = area.removeFromTop(50);
    auto tiltLabelArea = tiltArea.removeFromTop(20);
    tiltGainLabel->setBounds(tiltLabelArea.removeFromLeft(60));
    tiltValueLabel->setBounds(tiltLabelArea.removeFromRight(60));
    tiltGainDbSlider->setBounds(tiltArea.reduced(10, 0));

    area.removeFromTop(padding);

    // ========== Transient Shaper Section ==========
    auto transientArea = area.removeFromTop(160);

    // Band selector buttons at top
    auto bandButtonsArea = transientArea.removeFromTop(30);
    int buttonWidth = bandButtonsArea.getWidth() / 3;
    lowBandButton->setBounds(bandButtonsArea.removeFromLeft(buttonWidth));
    midBandButton->setBounds(bandButtonsArea.removeFromLeft(buttonWidth));
    highBandButton->setBounds(bandButtonsArea);

    transientArea.removeFromTop(padding);

    // Attack and Sustain sliders centered
    auto slidersArea = transientArea.removeFromTop(100);
    int sliderWidth = 70;
    int sliderSpacing = 40;

    int totalWidth = sliderWidth * 2 + sliderSpacing;
    slidersArea = slidersArea.withSizeKeepingCentre(totalWidth, slidersArea.getHeight());

    auto attackArea = slidersArea.removeFromLeft(sliderWidth);
    attackLabel->setBounds(attackArea.removeFromTop(15));
    attackValueLabel->setBounds(attackArea.removeFromBottom(15));
    transientAttackSlider->setBounds(attackArea.withSizeKeepingCentre(50, attackArea.getHeight()));

    slidersArea.removeFromLeft(sliderSpacing);

    auto sustainArea = slidersArea.removeFromLeft(sliderWidth);
    sustainLabel->setBounds(sustainArea.removeFromTop(15));
    sustainValueLabel->setBounds(sustainArea.removeFromBottom(15));
    transientSustainSlider->setBounds(sustainArea.withSizeKeepingCentre(50, sustainArea.getHeight()));

    // Smoothing ComboBox at bottom
    transientArea.removeFromTop(5);
    auto smoothingArea = transientArea.removeFromTop(25);
    smoothingLabel->setBounds(smoothingArea.removeFromLeft(80));
    smoothingComboBox->setBounds(smoothingArea.removeFromLeft(120).reduced(0, 2));

    // ========== Advanced Panel Toggle ==========
    area.removeFromTop(padding);
    auto toggleArea = area.removeFromTop(30);
    advancedToggleButton->setBounds(toggleArea);

    // ========== Advanced Parameters Panel (if shown) ==========
    if (showAdvancedPanel)
    {
        area.removeFromTop(padding);

        // First row: Tonality, Formant Base, Tilt Centre
        auto advancedRow1 = area.removeFromTop(120);
        int knobSize = 80;
        int advancedSpacing = (advancedRow1.getWidth() - knobSize * 3) / 4;

        auto tonalityArea = advancedRow1.removeFromLeft(advancedSpacing + knobSize);
        tonalityArea.removeFromLeft(advancedSpacing);
        tonalityHzLabel->setBounds(tonalityArea.removeFromTop(15));
        tonalityHzSlider->setBounds(tonalityArea.withSizeKeepingCentre(knobSize, knobSize + 20));

        auto formantBaseArea = advancedRow1.removeFromLeft(advancedSpacing + knobSize);
        formantBaseArea.removeFromLeft(advancedSpacing);
        formantBaseHzLabel->setBounds(formantBaseArea.removeFromTop(15));
        formantBaseHzSlider->setBounds(formantBaseArea.withSizeKeepingCentre(knobSize, knobSize + 20));

        auto tiltCentreArea = advancedRow1;
        tiltCentreArea.removeFromLeft(advancedSpacing);
        tiltCentreHzLabel->setBounds(tiltCentreArea.removeFromTop(15));
        tiltCentreHzSlider->setBounds(tiltCentreArea.withSizeKeepingCentre(knobSize, knobSize + 20));

        area.removeFromTop(padding);

        // Second row: Crossover frequencies
        auto advancedRow2 = area.removeFromTop(100);
        int xoverSize = 70;
        int xoverSpacing = (advancedRow2.getWidth() - xoverSize * 2) / 3;

        auto lowMidXoverArea = advancedRow2.removeFromLeft(xoverSpacing + xoverSize);
        lowMidXoverArea.removeFromLeft(xoverSpacing);
        lowMidXoverSlider->setBounds(lowMidXoverArea.removeFromTop(xoverSize).withSizeKeepingCentre(xoverSize, xoverSize));
        lowMidXoverLabel->setBounds(lowMidXoverArea);

        auto midHighXoverArea = advancedRow2.removeFromLeft(xoverSpacing + xoverSize);
        midHighXoverArea.removeFromLeft(xoverSpacing);
        midHighXoverSlider->setBounds(midHighXoverArea.removeFromTop(xoverSize).withSizeKeepingCentre(xoverSize, xoverSize));
        midHighXoverLabel->setBounds(midHighXoverArea);

        area.removeFromTop(padding);
        formantCompensationToggle->setBounds(area.removeFromTop(25));

        // Show advanced controls
        tonalityHzSlider->setVisible(true);
        tonalityHzLabel->setVisible(true);
        formantBaseHzSlider->setVisible(true);
        formantBaseHzLabel->setVisible(true);
        tiltCentreHzSlider->setVisible(true);
        tiltCentreHzLabel->setVisible(true);
        lowMidXoverSlider->setVisible(true);
        lowMidXoverLabel->setVisible(true);
        midHighXoverSlider->setVisible(true);
        midHighXoverLabel->setVisible(true);
        formantCompensationToggle->setVisible(true);

        setSize(400, 900);
    }
    else
    {
        // Hide advanced controls
        tonalityHzSlider->setVisible(false);
        tonalityHzLabel->setVisible(false);
        formantBaseHzSlider->setVisible(false);
        formantBaseHzLabel->setVisible(false);
        tiltCentreHzSlider->setVisible(false);
        tiltCentreHzLabel->setVisible(false);
        lowMidXoverSlider->setVisible(false);
        lowMidXoverLabel->setVisible(false);
        midHighXoverSlider->setVisible(false);
        midHighXoverLabel->setVisible(false);
        formantCompensationToggle->setVisible(false);

        setSize(400, 640);
    }
}

void SpectralShiftAudioProcessorEditor::switchToBand(int bandIndex)
{
    if (bandIndex == currentBand)
        return;

    currentBand = bandIndex;

    // Detach all attachments
    lowAttackAttachment.reset();
    lowSustainAttachment.reset();
    midAttackAttachment.reset();
    midSustainAttachment.reset();
    highAttackAttachment.reset();
    highSustainAttachment.reset();

    // Attach the selected band
    juce::Colour bandColour;
    switch (bandIndex)
    {
        case 0: // Low
            lowAttackAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "LOW_TRANS_ATTACK_DB", *transientAttackSlider);
            lowSustainAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "LOW_TRANS_SUSTAIN_DB", *transientSustainSlider);
            bandColour = CustomLookAndFeel::Colors::lowBand;
            break;
        case 1: // Mid
            midAttackAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "MID_TRANS_ATTACK_DB", *transientAttackSlider);
            midSustainAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "MID_TRANS_SUSTAIN_DB", *transientSustainSlider);
            bandColour = CustomLookAndFeel::Colors::midBand;
            break;
        case 2: // High
            highAttackAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "HIGH_TRANS_ATTACK_DB", *transientAttackSlider);
            highSustainAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "HIGH_TRANS_SUSTAIN_DB", *transientSustainSlider);
            bandColour = CustomLookAndFeel::Colors::highBand;
            break;
    }

    // Update slider colors
    transientAttackSlider->setColour(juce::Slider::trackColourId, bandColour);
    transientSustainSlider->setColour(juce::Slider::trackColourId, bandColour);
    attackValueLabel->setColour(juce::Label::textColourId, bandColour);
    sustainValueLabel->setColour(juce::Label::textColourId, bandColour);

    // Trigger value labels update
    attackValueLabel->setText(juce::String(transientAttackSlider->getValue(), 1) + " dB", juce::dontSendNotification);
    sustainValueLabel->setText(juce::String(transientSustainSlider->getValue(), 1) + " dB", juce::dontSendNotification);

    repaint();
}