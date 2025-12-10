/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginTemplateAudioProcessorEditor::PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Volume Slider
    driveSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    addAndMakeVisible(driveSlider.get());

    // Volume Label
    driveLabel = std::make_unique<juce::Label>("", "Drive");
    addAndMakeVisible(driveLabel.get());
    driveLabel->attachToComponent(driveSlider.get(), false);
    driveLabel->setJustificationType(juce::Justification::centred);

    // Volume Slider
    volSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    addAndMakeVisible(volSlider.get());

    // Volume Label
    volLabel = std::make_unique<juce::Label>("", "Volume");
    addAndMakeVisible(volLabel.get());
    volLabel->attachToComponent(volSlider.get(), false);
    volLabel->setJustificationType(juce::Justification::centred);

    // mix Slider
    mixSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    addAndMakeVisible(mixSlider.get());

    // mix Label
    mixLabel = std::make_unique<juce::Label>("", "Wet/Dry");
    addAndMakeVisible(mixLabel.get());
    mixLabel->attachToComponent(mixSlider.get(), false);
    mixLabel->setJustificationType(juce::Justification::centred);

    // Attachments
    //using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    driveAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "DRIVE", *driveSlider);
    volAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "VOL", *volSlider);
    mixAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "MIX", *mixSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

PluginTemplateAudioProcessorEditor::~PluginTemplateAudioProcessorEditor()
{
}

//==============================================================================
void PluginTemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto textBounds = bounds.removeFromTop(40);

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(bounds);

    g.setColour(juce::Colours::black);
    g.fillRect(textBounds);

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("PluginTemplate", textBounds, juce::Justification::centred, 1);
}

void PluginTemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

        // bounds is the "background" of our plug-in
    auto bounds = getLocalBounds();

    // Give 40 pixels of space for the title
    auto textBounds = bounds.removeFromTop(40);

    // Reserve some room for the gain meter
    //bounds.removeFromRight(40);

    // Give 40 pixels of margin on all sides so we don't paint to the edges of the background
    bounds.reduce(40, 40);

    juce::Grid grid;

    // TrackInfo is a "space"
    using Track = juce::Grid::TrackInfo;

    // Fr is how big the space is relative to other spaces
    using Fr = juce::Grid::Fr;

    // 4 columns x 2 rows
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.templateRows = { Track(Fr(1)), Track(Fr(1)) };

    // Margin between each space
    grid.rowGap = juce::Grid::Px(20);
    grid.columnGap = juce::Grid::Px(20);

    // Add components to the grid
    grid.items.add(driveSlider.get());
    grid.items.add(volSlider.get());
    grid.items.add(mixSlider.get());


    // Perform the layout on the bounds
    grid.performLayout(bounds);
}