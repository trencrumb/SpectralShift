/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectralShiftAudioProcessor::SpectralShiftAudioProcessor()
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
    apvts.state.addListener(this);
    init();
}

SpectralShiftAudioProcessor::~SpectralShiftAudioProcessor()
{
    stretch.presetDefault(getTotalNumInputChannels(), getSampleRate(), false);
}

//==============================================================================
const juce::String SpectralShiftAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectralShiftAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectralShiftAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectralShiftAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectralShiftAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectralShiftAudioProcessor::getNumPrograms()
{
        return (int) presets.size();
   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectralShiftAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex < 0 ? 0 : currentPresetIndex;
}

void SpectralShiftAudioProcessor::setCurrentProgram (int index)
{
    setPreset (index);

}

const juce::String SpectralShiftAudioProcessor::getProgramName (int index)
{
    if (index < 0 || index >= (int) presets.size())
        return {};

    return presets[(size_t) index].name;
}


void SpectralShiftAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    if (index < 0 || index >= (int) presets.size())
        return;

    presets[(size_t) index].name = newName;
}


//==============================================================================
void SpectralShiftAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    isActive = true;

    const int channels = getTotalNumInputChannels();

    stretch.presetDefault(channels, static_cast<int>(sampleRate), true);
    stretch.reset();

    stretchBuffer.setSize(channels, samplesPerBlock);
    inPtrs.resize(channels);
    outPtrs.resize(channels);

    const int inputLatency  = stretch.inputLatency();
    const int outputLatency = stretch.outputLatency();
    setLatencySamples(inputLatency + outputLatency);

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = channels;
    tiltEQ.prepare(spec);
    monoBuffer.resize(samplesPerBlock);
    spectralCentroid.prepare(sampleRate, samplesPerBlock);

    // Reset CPU load measurer with current sample rate
    loadMeasurer.reset(sampleRate, samplesPerBlock);

    prepare(sampleRate, samplesPerBlock);
    update();
    reset();
}


void SpectralShiftAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectralShiftAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpectralShiftAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& midiMessages)
{
    #if PERFETTO
        TRACE_DSP();  // Perfetto trace for entire processBlock
    #endif


    juce::ignoreUnused(midiMessages);

    if (! isActive)
        return;

    // Measure CPU load - this scoped timer automatically tracks the processing time
    const juce::AudioProcessLoadMeasurer::ScopedTimer timer(loadMeasurer, buffer.getNumSamples());
    #if PERFETTO
    TRACE_EVENT_BEGIN("dsp", "parameter-update");
    #endif
    update();
    #if PERFETTO
    TRACE_EVENT_END("dsp");
    #endif


    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    stretchBuffer.setSize (numChannels, numSamples, false, false, true);


    if (numChannels == 0 || numSamples == 0)
        return;

    //const double sr = getSampleRate();

    //float tonalityLimitNorm = 0.0f;
    //float formantBaseNorm   = 0.0f;

    //if (sr > 0.0)
    //{
    //    tonalityLimitNorm = juce::jlimit (0.0f, 0.5f, currentTonalityHz   / (float) sr);
    //    formantBaseNorm   = juce::jlimit (0.0f, 0.5f, currentFormantBaseHz / (float) sr);
    //}

    const float pitchSemitones       = currentPitchSemitones;
    const float formantSemitones     = currentFormantSemitones;
    const bool  formantCompensation  = currentFormantPreservation;

    const float tonalityHz     = currentTonalityHz;
    const float formantBaseHz  = currentFormantBaseHz;

    const float tiltGainDB   = currentTiltGainDB;

    const float sr = (float) getSampleRate();

    float tonalityLimitNorm = 0.0f;
    float formantBaseNorm   = 0.0f;

    if (sr > 0.0f)
    {
        // cycles/sample (0..0.5 is 0..Nyquist)
        tonalityLimitNorm = juce::jlimit (0.0f, 0.5f, tonalityHz    / sr);
    }

    float safeFormantBaseHz = 0.0f;
    if (formantBaseHz > 0.0f)
        safeFormantBaseHz = juce::jlimit (20.0f, 2000.0f, formantBaseHz);


    stretch.setTransposeSemitones (pitchSemitones, tonalityLimitNorm);
    stretch.setFormantSemitones   (formantSemitones, formantCompensation);
    stretch.setFormantBase        (safeFormantBaseHz);

    // Tilt EQ center frequency is now set automatically by spectral centroid (see below)
    tiltEQ.setGainDb(tiltGainDB);



    // --- 2. Prepare input/output pointer arrays ---

    for (int ch = 0; ch < numChannels; ++ch)
    {
        inPtrs[ch]  = const_cast<std::vector<float *>::value_type>(buffer.getReadPointer(ch));
        outPtrs[ch] = stretchBuffer.getWritePointer (ch);
    }


    int inputSamples  = numSamples;
    int outputSamples = numSamples;

    // --- 3. Process with Signalsmith Stretch ---
    #if PERFETTO
    TRACE_EVENT_BEGIN("dsp", "signalsmith-stretch");
    #endif

    stretch.process(inPtrs.data(), inputSamples,
                    outPtrs.data(), outputSamples);
#if PERFETTO
    TRACE_EVENT_END("dsp");
#endif

    // outputSamples may differ slightly; be safe when copying back
    const int copySamples = std::min(numSamples, outputSamples);

    // --- 4. Copy processed audio back into JUCE buffer ---
    #if PERFETTO
    TRACE_EVENT_BEGIN("dsp", "buffer-copy");
    #endif
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.clear(ch, 0, numSamples);
        buffer.copyFrom(ch, 0, stretchBuffer, ch, 0, copySamples);

    }
    #if PERFETTO
    TRACE_EVENT_END("dsp");
    TRACE_EVENT_BEGIN("dsp", "mono-sum");
    #endif
    monoBuffer.resize(numSamples);  // Only resizes if different
    std::fill(monoBuffer.begin(), monoBuffer.end(), 0.0f);


    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* x = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            monoBuffer[(size_t) i] += x[i];
    }

    const float invCh = 1.0f / (float) numChannels;
    for (int i = 0; i < numSamples; ++i)
        monoBuffer[(size_t) i] *= invCh;
    #if PERFETTO
    TRACE_EVENT_END("dsp");

    TRACE_EVENT_BEGIN("dsp", "tilt-centre-calculation");
    #endif
    auto* tiltCentreAutoParam = apvts.getRawParameterValue("TILT_CENTRE_AUTO");
    bool isAuto = tiltCentreAutoParam->load() > 0.5f;

    float tiltCentreHz;
    if (isAuto) {
#if PERFETTO
        TRACE_EVENT_BEGIN("dsp", "spectral-centroid");
        #endif
        // Use spectral centroid
        spectralCentroid.processBlock(monoBuffer.data(), numSamples);
        tiltCentreHz = spectralCentroid.getCentroidHz();
        #if PERFETTO
        TRACE_EVENT_END("dsp");
        #endif

        // Clamp to parameter range (200-20000 Hz) before updating slider
        tiltCentreHz = juce::jlimit(200.0f, 20000.0f, tiltCentreHz);

        // Update the slider parameter value for visualization (without triggering listeners)
        if (auto* param = apvts.getParameter("TILT_CENTRE_HZ"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(tiltCentreHz));
    } else {
        auto* manualParam = apvts.getRawParameterValue("TILT_CENTRE_HZ");
        tiltCentreHz = manualParam->load();
    }

    tiltEQ.setCentreFrequency(tiltCentreHz);
    #if PERFETTO
    TRACE_EVENT_END("dsp");

    TRACE_EVENT_BEGIN("dsp", "tilt-eq");
    #endif
    tiltEQ.process (buffer);
    #if PERFETTO
    TRACE_EVENT_END("dsp");
    #endif
}


//==============================================================================
bool SpectralShiftAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectralShiftAudioProcessor::createEditor()
{
    return new SpectralShiftAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);

}

//==============================================================================
void SpectralShiftAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
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

void SpectralShiftAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

void SpectralShiftAudioProcessor::init()
{
    presets = {
        {
            "Subtle Brighten",
            {
                    { "PITCH_SEMITONES",   2.0f },
                    { "PITCH_CENTS",       0.0f },
                    { "FORMANT_SEMITONES", 1.0f },
                    { "FORMANT_CENTS",     0.0f },
                    { "FORMANT_COMPENSATION", 1.0f },
                    { "TONALITY_HZ",       6000.0f },
                    { "FORMANT_BASE_HZ",   200.0f },
                    { "TILT_GAIN_DB",      1.5f },
                }
        },
        {
            "Thick Low",
            {
                    { "PITCH_SEMITONES",   7.0f },
                    { "PITCH_CENTS",       0.0f },
                    { "FORMANT_SEMITONES", -5.0f },
                    { "FORMANT_CENTS",     0.0f },
                    { "FORMANT_COMPENSATION", 1.0f },
                    { "TONALITY_HZ",       3000.0f },
                    { "FORMANT_BASE_HZ",   150.0f },
                    { "TILT_GAIN_DB",     -2.0f },
                }
        }
    };
}

void SpectralShiftAudioProcessor::prepare(double sampleRate, int samplesPerBlock)
{
}

void SpectralShiftAudioProcessor::setPreset (int index)
{
    if (index < 0 || index >= (int) presets.size())
        return;

    const auto& preset = presets[(size_t) index];

    juce::ScopedValueSetter<bool> svs (mustUpdateProcessing, true, false);

    for (const auto& [paramID, value] : preset.values)
    {
        if (auto* param = apvts.getParameter (paramID))
            param->setValueNotifyingHost (param->getNormalisableRange().convertTo0to1 (value));
    }

    currentPresetIndex = index;
}


void SpectralShiftAudioProcessor::update()
{
    mustUpdateProcessing = false;
    auto* pitchSemiParam   = apvts.getRawParameterValue("PITCH_SEMITONES");
    auto* pitchCentsParam  = apvts.getRawParameterValue("PITCH_CENTS");
    auto* formSemiParam    = apvts.getRawParameterValue("FORMANT_SEMITONES");
    auto* formCentsParam   = apvts.getRawParameterValue("FORMANT_CENTS");
    auto* formCompParam    = apvts.getRawParameterValue("FORMANT_COMPENSATION");
    auto* tonalityHzParam  = apvts.getRawParameterValue("TONALITY_HZ");
    auto* formantBaseParam = apvts.getRawParameterValue("FORMANT_BASE_HZ");

    auto* tiltGainDBParam = apvts.getRawParameterValue("TILT_GAIN_DB");


    const float pitchSemi  = pitchSemiParam->load();
    const float pitchCents = pitchCentsParam->load();
    const float formSemi   = formSemiParam->load();
    const float formCents  = formCentsParam->load();

    currentPitchSemitones   = pitchSemi + pitchCents / 100.0f;
    currentFormantSemitones = formSemi  + formCents  / 100.0f;

    currentFormantPreservation = (formCompParam->load() >= 0.5f);
    currentTonalityHz          = tonalityHzParam->load();
    currentFormantBaseHz       = formantBaseParam->load();

    currentTiltGainDB          = tiltGainDBParam->load();
}

void SpectralShiftAudioProcessor::reset()
{
    stretch.reset();
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectralShiftAudioProcessor::createParameters()
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

    // Pitch in semitones
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "PITCH_SEMITONES", "Pitch (semitones)",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f, "st",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Pitch in cents
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "PITCH_CENTS", "Pitch (cents)",
        juce::NormalisableRange<float>(-200.0f, 200.0f, 1.0f), 0.0f, "c",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Formant in semitones
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FORMANT_SEMITONES", "Formant (semitones)",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f, "st",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Formant in cents
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FORMANT_CENTS", "Formant (cents)",
        juce::NormalisableRange<float>(-200.0f, 200.0f, 1.0f), 0.0f, "c",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "FORMANT_COMPENSATION", "Formant Compensation", true));

    // Tonality Hz - logarithmic range centered at 2kHz
    auto tonalityRange = juce::NormalisableRange<float>(200.0f, 20000.0f, 10.0f);
    tonalityRange.setSkewForCentre(2000.0f);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TONALITY_HZ", "Tonality Hz",
        tonalityRange, 5000.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FORMANT_BASE_HZ", "Formant Base Hz",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f), 0.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    auto tiltGainRange = juce::NormalisableRange<float>(-6.0f, 6.0f, 0.001f);
    tiltGainRange.setSkewForCentre(0.0f);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TILT_GAIN_DB", "Tilt Gain db",
        tiltGainRange, 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Tilt Centre Hz - logarithmic range centered at 1kHz (extended to 20kHz for high sample rates)
    auto tiltCentreRange = juce::NormalisableRange<float>(200.0f, 20000.0f, 5.0f);
    tiltCentreRange.setSkewForCentre(1000.0f);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TILT_CENTRE_HZ",
        "Tilt Centre Hz",
        tiltCentreRange,
        1000.0f));

    // Auto tilt centre toggle (default ON)
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "TILT_CENTRE_AUTO",
        "Tilt Centre Auto",
        true));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectralShiftAudioProcessor();
}

