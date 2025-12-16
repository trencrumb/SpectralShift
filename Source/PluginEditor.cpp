#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectralShiftAudioProcessorEditor::SpectralShiftAudioProcessorEditor (SpectralShiftAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    const bool editableTextLabels = true;

    addAndMakeVisible(xyPad);

    // ================== Pitch (semitones) ==================
    pitchSemitonesSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    );
    pitchSemitonesSlider->setRange(-12.0, 12.0, 0.01);
    pitchSemitonesSlider->setTextValueSuffix(" st");
    pitchSemitonesSlider->setNumDecimalPlacesToDisplay(2);
    pitchSemitonesSlider->setTextBoxIsEditable(editableTextLabels);
    pitchSemitonesSlider->setTooltip("Pitch shift in semitones (coarse)");
    pitchSemitonesSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*pitchSemitonesSlider);


    // ================== Pitch (cents) ==================
    pitchCentsSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    );
    pitchCentsSlider->setRange(-200.0, 200.0, 1.0);
    pitchCentsSlider->setTextValueSuffix(" c");
    pitchCentsSlider->setNumDecimalPlacesToDisplay(0);
    pitchCentsSlider->setTextBoxIsEditable(editableTextLabels);
    pitchCentsSlider->setTooltip("Pitch fine-tuning in cents (±100 per semitone)");
    pitchCentsSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*pitchCentsSlider);

    // ================== Formant (semitones) ==================
    formantSemitonesSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    );
    formantSemitonesSlider->setRange(-12.0, 12.0, 0.01);
    formantSemitonesSlider->setTextValueSuffix(" st");
    formantSemitonesSlider->setNumDecimalPlacesToDisplay(2);
    formantSemitonesSlider->setTextBoxIsEditable(editableTextLabels);
    formantSemitonesSlider->setTooltip("Formant shift in semitones (coarse)");
    formantSemitonesSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*formantSemitonesSlider);


    // ================== Formant (cents) ==================
    formantCentsSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    );
    formantCentsSlider->setRange(-200.0, 200.0, 1.0);
    formantCentsSlider->setTextValueSuffix(" c");
    formantCentsSlider->setNumDecimalPlacesToDisplay(0);
    formantCentsSlider->setTextBoxIsEditable(editableTextLabels);
    formantCentsSlider->setTooltip("Formant fine-tuning in cents");
    formantCentsSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*formantCentsSlider);

    // ================== Formant Settings ==================

    formantCompensationToggle = std::make_unique<juce::ToggleButton>(
        "Formant Compensation"
    );
    addAndMakeVisible(*formantCompensationToggle);

    // ================== Transient Settings ==============

    transientAttackDBSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
        );
    transientAttackDBSlider->setTextValueSuffix(" dB");
    transientAttackDBSlider->setNumDecimalPlacesToDisplay(2);
    transientAttackDBSlider->setTextBoxIsEditable(editableTextLabels);
    transientAttackDBSlider->setTooltip("Transient attack amount (dB)");
    transientAttackDBSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*transientAttackDBSlider);


    transientSustainDBSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
        );
    transientSustainDBSlider->setTextValueSuffix(" dB");
    transientSustainDBSlider->setNumDecimalPlacesToDisplay(2);
    transientSustainDBSlider->setTextBoxIsEditable(editableTextLabels);
    transientSustainDBSlider->setTooltip("Transient sustain amount (dB)");
    transientSustainDBSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*transientSustainDBSlider);


    // ================== XY Pad bindings ==================
    xyPad.registerSlider(pitchSemitonesSlider.get(), XYPad::Axis::X);
    xyPad.registerSlider(formantSemitonesSlider.get(), XYPad::Axis::Y);

    // ================== Stretch Settings =================
    formantBaseHzSlider = std::make_unique<juce::Slider>(
        juce::Slider::LinearHorizontal,
        juce::Slider::TextBoxBelow
    );
    formantBaseHzSlider->setRange(0.0, 500.0, 1.0);
    formantBaseHzSlider->setTextValueSuffix(" Hz");
    formantBaseHzSlider->setNumDecimalPlacesToDisplay(2);
    formantBaseHzSlider->setTextBoxIsEditable(editableTextLabels);
    formantBaseHzSlider->setTooltip("Formant base frequency for stretching");
    addAndMakeVisible(*formantBaseHzSlider);

    tonalityHzSlider = std::make_unique<juce::Slider>(
        juce::Slider::LinearHorizontal,
        juce::Slider::TextBoxBelow
        );
    tonalityHzSlider->setRange(0.0, 500.0, 1.0);
    tonalityHzSlider->setTextValueSuffix(" Hz");
    tonalityHzSlider->setNumDecimalPlacesToDisplay(2);
    tonalityHzSlider->setTextBoxIsEditable(editableTextLabels);
    tonalityHzSlider->setTooltip("Tonality analysis limit (Hz)");
    addAndMakeVisible(*tonalityHzSlider);

    // ================ Tilt Controls ==================
    tiltCentreHzSlider = std::make_unique<juce::Slider>(
        juce::Slider::LinearHorizontal,
        juce::Slider::TextBoxBelow
    );
    // match processor parameter: 20..20000 (int)
    tiltCentreHzSlider->setRange(20.0, 20000.0, 1.0);
    tiltCentreHzSlider->setTextValueSuffix(" Hz");
    tiltCentreHzSlider->setNumDecimalPlacesToDisplay(0);
    tiltCentreHzSlider->setTextBoxIsEditable(editableTextLabels);
    tiltCentreHzSlider->setTooltip("Tilt centre frequency (Hz)");
    addAndMakeVisible(*tiltCentreHzSlider);


    tiltGainDbSlider = std::make_unique<juce::Slider>(
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    );
    tiltGainDbSlider->setRange(-6.0, 6.0, 0.01);
    tiltGainDbSlider->setTextValueSuffix(" dB");
    tiltGainDbSlider->setNumDecimalPlacesToDisplay(2);
    tiltGainDbSlider->setTextBoxIsEditable(editableTextLabels);
    tiltGainDbSlider->setTextBoxIsEditable(editableTextLabels);
    tiltGainDbSlider->setTooltip("Tilt gain in dB (slope around centre)");
    tiltGainDbSlider->setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    addAndMakeVisible(*tiltGainDbSlider);

    // Create labels for tilt controls
    tiltCentreHzLabel = std::make_unique<juce::Label>("", "Tilt Centre");
    addAndMakeVisible(*tiltCentreHzLabel);
    tiltCentreHzLabel->attachToComponent(tiltCentreHzSlider.get(), false);
    tiltCentreHzLabel->setJustificationType(juce::Justification::centred);

    tiltGainDbLabel = std::make_unique<juce::Label>("", "Tilt Gain");
    addAndMakeVisible(*tiltGainDbLabel);
    tiltGainDbLabel->attachToComponent(tiltGainDbSlider.get(), false);
    tiltGainDbLabel->setJustificationType(juce::Justification::centred);

    // ...attachments
    pitchSemitonesAttachment =
        std::make_unique<Attachment>(audioProcessor.apvts,
                                     "PITCH_SEMITONES",
                                     *pitchSemitonesSlider);
    pitchCentsAttachment =
        std::make_unique<Attachment>(audioProcessor.apvts,
                                     "PITCH_CENTS",
                                     *pitchCentsSlider);
    formantSemitonesAttachment =
        std::make_unique<Attachment>(audioProcessor.apvts,
                                     "FORMANT_SEMITONES",
                                     *formantSemitonesSlider);
    formantCentsAttachment =
        std::make_unique<Attachment>(audioProcessor.apvts,
                                     "FORMANT_CENTS",
                                     *formantCentsSlider);
    formantBaseHzAttachment =
        std::make_unique<Attachment>(audioProcessor.apvts,
                                     "FORMANT_BASE_HZ",
                                     *formantBaseHzSlider);

    formantCompensationAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            audioProcessor.apvts,
            "FORMANT_COMPENSATION",
            *formantCompensationToggle);

    tonalityHzAttachment =
        std::make_unique<Attachment>(audioProcessor.apvts,
                                     "TONALITY_HZ",
                                     *tonalityHzSlider);

    // Attach transient params
    transientAttackDBAttachment = std::make_unique<Attachment>(
        audioProcessor.apvts,
        "TRANS_ATTACK_DB",
        *transientAttackDBSlider);

    transientSustainDBAttachment = std::make_unique<Attachment>(
        audioProcessor.apvts,
        "TRANS_SUSTAIN_DB",
        *transientSustainDBSlider);

    // Attach tilt params
    tiltCentreHzAttachment = std::make_unique<Attachment>(
        audioProcessor.apvts,
        "TILT_CENTRE_HZ",
        *tiltCentreHzSlider);

    tiltGainDbAttachment = std::make_unique<Attachment>(
        audioProcessor.apvts,
        "TILT_GAIN_DB",
        *tiltGainDbSlider);

    setSize (700, 650);
}

SpectralShiftAudioProcessorEditor::~SpectralShiftAudioProcessorEditor()
{
    xyPad.deregisterSlider(pitchSemitonesSlider.get());
    xyPad.deregisterSlider(formantSemitonesSlider.get());
}

//==============================================================================
void SpectralShiftAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel()
                   .findColour (juce::ResizableWindow::backgroundColourId));
}

void SpectralShiftAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Reserve a fixed bottom strip for secondary controls
    const int bottomHeight = juce::jmax(60, area.getHeight() / 5);
    auto bottomArea = area.removeFromBottom(bottomHeight);

    // Columns for left (pitch), center (XY pad), right (formant)
    const auto leftWidth  = area.proportionOfWidth(0.20f);
    const auto padWidth   = area.proportionOfWidth(0.60f);
    auto leftArea  = area.removeFromLeft(leftWidth);
    auto padArea   = area.removeFromLeft(padWidth);
    auto rightArea = area; // remaining area is right column

    // Make the pad a centred square inside padArea so X/Y map consistently
    int padSize = juce::jmin(padArea.getWidth(), padArea.getHeight());
    juce::Rectangle<int> squarePad = padArea.withSizeKeepingCentre(padSize, padSize);
    xyPad.setBounds(squarePad.reduced(10));

    // --- Use juce::Grid for left column (semitones above cents) ---
    if (pitchSemitonesSlider && pitchCentsSlider)
    {
        juce::Grid leftGrid;
        leftGrid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        leftGrid.templateRows    = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                     juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        leftGrid.items.add(juce::GridItem(*pitchSemitonesSlider));
        leftGrid.items.add(juce::GridItem(*pitchCentsSlider));
        leftGrid.performLayout(leftArea.reduced(6));
    }
    else if (pitchSemitonesSlider) // fallback single
    {
        pitchSemitonesSlider->setBounds(leftArea.reduced(6));
    }

    // --- Use juce::Grid for right column (semitones above cents) ---
    if (formantSemitonesSlider && formantCentsSlider)
    {
        juce::Grid rightGrid;
        rightGrid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        rightGrid.templateRows    = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                      juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        rightGrid.items.add(juce::GridItem(*formantSemitonesSlider));
        rightGrid.items.add(juce::GridItem(*formantCentsSlider));
        rightGrid.performLayout(rightArea.reduced(6));
    }
    else if (formantSemitonesSlider)
    {
        formantSemitonesSlider->setBounds(rightArea.reduced(6));
    }

    // Bottom column: place available sliders stacked vertically (full width)
    std::vector<juce::Slider*> bottomSliders = {
        formantBaseHzSlider.get(),
        tonalityHzSlider.get(),
        tiltCentreHzSlider.get(),
        tiltGainDbSlider.get()
    };

    int available = 0;
    for (auto s : bottomSliders) if (s) ++available;
    if (available == 0)
        return;

    const int rowHeight = bottomArea.getHeight() / available;
    auto rowArea = bottomArea;

    for (auto s : bottomSliders)
    {
        if (!s) continue;
        s->setBounds(rowArea.removeFromTop(rowHeight).reduced(6));
    }
}

