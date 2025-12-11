/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginTemplateAudioProcessor::PluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)

                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
    apvts.state.addListener(this); // [2] Add this too!
    init();
}

PluginTemplateAudioProcessor::~PluginTemplateAudioProcessor()
{
    stretch.presetDefault(getTotalNumInputChannels(), getSampleRate(), false);
}

//==============================================================================
const juce::String PluginTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginTemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void PluginTemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    isActive = true;

    const int channels = getTotalNumInputChannels();

    // Configure Signalsmith Stretch
    stretch.presetDefault(channels, (int) sampleRate);
    stretch.reset();

    // Optional but recommended: inform the host about latency
    const int inputLatency  = stretch.inputLatency();
    const int outputLatency = stretch.outputLatency();
    setLatencySamples(inputLatency + outputLatency);

    prepare(sampleRate, samplesPerBlock);
    update();
    reset();
}


void PluginTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PluginTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    if (! isActive)
        return;

    if (mustUpdateProcessing)
        update();

    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    // --- 1. Set pitch + formant (tonality limit) ---

    // Example: use cached parameter values, or replace with constants
    const float pitchSemitones     = currentPitchSemitones; // e.g. -12..+12
    const float formantSemitones = currentFormantSemitones;  // 0..1
    const bool formantCompensation = currentFormantPreservation;
    const float tonalityLimit = static_cast<float>(currentTonalityHz / getSampleRate());
    const float formantBaseHz = currentFormantBaseHz;

    const double sampleRate   = getSampleRate();

    stretch.setTransposeSemitones(pitchSemitones, tonalityLimit);
    stretch.setFormantSemitones(12, true);
    stretch.setFormantBase(formantBaseHz);
    // JS implementation
    //wasmModule._setTransposeSemitones(currentMapSegment.semitones, currentMapSegment.tonalityHz/sampleRate);
    //wasmModule._setFormantSemitones(currentMapSegment.formantSemitones, currentMapSegment.formantCompensation);
    //wasmModule._setFormantBase(currentMapSegment.formantBaseHz/sampleRate);

    // --- 2. Prepare input/output pointer arrays ---

    // Input pointers from JUCE buffer
    std::vector<float*> inPtrs(numChannels);
    for (int ch = 0; ch < numChannels; ++ch)
        inPtrs[ch] = buffer.getWritePointer(ch); // or getReadPointer if you don't need in‑place mods after

    // Temporary output buffer for processed audio
    juce::AudioBuffer<float> outBuffer(numChannels, numSamples);
    std::vector<float*> outPtrs(numChannels);
    for (int ch = 0; ch < numChannels; ++ch)
        outPtrs[ch] = outBuffer.getWritePointer(ch);

    int inputSamples  = numSamples;
    int outputSamples = numSamples; // ask for same length: we only want pitch shift

    // --- 3. Process with Signalsmith Stretch ---

    stretch.process(inPtrs.data(), inputSamples,
                    outPtrs.data(), outputSamples);

    // outputSamples may differ slightly; be safe when copying back
    const int copySamples = std::min(numSamples, outputSamples);

    // --- 4. Copy processed audio back into JUCE buffer ---

    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.clear(ch, 0, numSamples);
        buffer.copyFrom(ch, 0, outBuffer, ch, 0, copySamples);
    }
}


//==============================================================================
bool PluginTemplateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginTemplateAudioProcessor::createEditor()
{
    //return new PluginTemplateAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PluginTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // Create a temporary ValueTree object called copyState and assign it the value returned by apvts.copyState();
    juce::ValueTree copyState = apvts.copyState();

    // Create a unique_ptr to copy XML information
    std::unique_ptr<juce::XmlElement> xml = copyState.createXml();

    // Copy XML that we just created to our binary (Our Memory block)
    copyXmlToBinary(*xml.get(), destData);
}

void PluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Create an xml pointer, and get the XML from the binary (our memory block)
    std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary(data, sizeInBytes);

    // Create a temporary ValueTree called copyState and save that data into our ValueTree object
    juce::ValueTree copyState = juce::ValueTree::fromXml(*xml.get());

    // Now we will replace the state with our copyState object in our apvts object
    apvts.replaceState(copyState);
}

void PluginTemplateAudioProcessor::init()
{
}

void PluginTemplateAudioProcessor::prepare(double sampleRate, int samplesPerBlock)
{
}

void PluginTemplateAudioProcessor::update()
{
    mustUpdateProcessing = false;

    //Load variables from APVTS
    //auto drive = apvts.getRawParameterValue("DRIVE");
    //auto volume = apvts.getRawParameterValue("VOL");
    //auto mix = apvts.getRawParameterValue("MIX");
    auto* pitchParam   = apvts.getRawParameterValue("PITCH");
    auto* formantParam = apvts.getRawParameterValue("FORMANT");
    auto* formantCompensation = apvts.getRawParameterValue("FORMANT_COMPENSATION");
    auto* tonalityHz = apvts.getRawParameterValue("TONALITY_HZ");
    auto* formantBaseHz = apvts.getRawParameterValue("FORMANT_BASE_HZ");


    //driveNormal = drive->load();
    currentPitchSemitones = pitchParam->load();
    currentFormantSemitones  = formantParam->load();
    currentFormantPreservation = static_cast<bool>(formantCompensation->load());
    currentTonalityHz = tonalityHz->load();
    currentFormantBaseHz = formantBaseHz->load();


    for (int channel = 0; channel < 2; ++channel)
    {
        //outputVolume[channel].setTargetValue(juce::Decibels::decibelsToGain(volume->load()));
        //outputMix[channel].setTargetValue(mix->load());
    }

}

void PluginTemplateAudioProcessor::reset()
{
    driveNormal.reset(getSampleRate(), 0.050);

    for (int channel = 0; channel < 2; ++channel)
    {
        // reset(sampleRate, rampLength in seconds)
        outputVolume[channel].reset(getSampleRate(), 0.001);
        outputMix[channel].reset(getSampleRate(), 0.001);
    }
    stretch.reset();
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginTemplateAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    // Creates a function takes floats/ints and returns a string
    std::function<juce::String(float, int)> valueToTextFunction = [](float x, int l) { return juce::String(x, 4); };

    // Creates a function that takes a String and returns a float
    std::function<float(const juce::String&)> textToValueFunction = [](const juce::String& str) { return str.getFloatValue(); };

    // Add a Drive Parameter to our vector of parameters
    //parameters.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f, "%", juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Add a Volume Parameter to our vector of parameters
    //parameters.push_back(std::make_unique<juce::AudioParameterFloat>("VOL", "Volume", juce::NormalisableRange<float>(-40.0f, 40.0f), 0.0f, "db",
    //    juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Add a Wet/Dry Parameter to our vector of parameters
    //parameters.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f, "%",
    //    juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "PITCH", "Pitch",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f, "semitones",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FORMANT", "Formant",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f, "semitones",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "FORMANT_COMPENSATION", "Formant Compensation", true));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TONALITY_HZ", "Tonality Hz",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f), 8000.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FORMANT_BASE_HZ", "Formant Base Hz",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f), 200.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));




    return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginTemplateAudioProcessor();
}