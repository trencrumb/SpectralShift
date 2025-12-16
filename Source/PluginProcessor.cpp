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
    for (auto& mbShaper : multibandTransientShaper)
        mbShaper.prepare(sampleRate, samplesPerBlock);

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
    juce::ignoreUnused(midiMessages);

    if (! isActive)
        return;

    // Always refresh parameters each block so GUI/host changes (including generic editor) are applied immediately.
    update();

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

    const float tiltCentreHz = currentTiltCentreHz;
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


    tiltEQ.setCentreFrequency(tiltCentreHz);
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

    stretch.process(inPtrs.data(), inputSamples,
                    outPtrs.data(), outputSamples);

    // outputSamples may differ slightly; be safe when copying back
    const int copySamples = std::min(numSamples, outputSamples);

    // --- 4. Copy processed audio back into JUCE buffer ---

    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.clear(ch, 0, numSamples);
        buffer.copyFrom(ch, 0, stretchBuffer, ch, 0, copySamples);

    }

    std::vector<float> mono;

    mono.resize((size_t) numSamples);
    std::fill(mono.begin(), mono.end(), 0.0f);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* x = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            mono[(size_t) i] += x[i];
    }

    const float invCh = 1.0f / (float) numChannels;
    for (int i = 0; i < numSamples; ++i)
        mono[(size_t) i] *= invCh;

    // 2) Estimate f0 (pick sensible min/max for your use-case)
    const float f0 = estimatePitchHzAutocorr(mono.data(), numSamples, sr, 50.0f, 500.0f);

    if (f0 > 0.0f)
    {
        float targetTiltCentreHz = juce::jlimit(20.0f, 20000.0f, f0 * 8.0f);

        // 4) Smooth it (recommended to avoid chatter)
        // e.g. use juce::SmoothedValue<float> as a member and call setTargetValue()
        // then in your sample loop: smoothed.getNextValue()
        tiltEQ.setCentreFrequency(targetTiltCentreHz);
        currentTiltCentreHz = targetTiltCentreHz;
    }



    tiltEQ.process (buffer);

    // Apply multiband transient shaper to the final buffer to shape attack/sustain

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = multibandTransientShaper[ch].processSample(data[i]);
    }

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
                    { "TILT_CENTRE_HZ",    4000.0f },
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
                    { "TILT_CENTRE_HZ",    1000.0f },
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

    // Multiband transient shaper parameters
    auto* lowAttackParam = apvts.getRawParameterValue("LOW_TRANS_ATTACK_DB");
    auto* lowSustainParam = apvts.getRawParameterValue("LOW_TRANS_SUSTAIN_DB");
    auto* midAttackParam = apvts.getRawParameterValue("MID_TRANS_ATTACK_DB");
    auto* midSustainParam = apvts.getRawParameterValue("MID_TRANS_SUSTAIN_DB");
    auto* highAttackParam = apvts.getRawParameterValue("HIGH_TRANS_ATTACK_DB");
    auto* highSustainParam = apvts.getRawParameterValue("HIGH_TRANS_SUSTAIN_DB");
    auto* lowMidXoverParam = apvts.getRawParameterValue("TRANS_LOWMID_XOVER_HZ");
    auto* midHighXoverParam = apvts.getRawParameterValue("TRANS_MIDHIGH_XOVER_HZ");

    auto* tiltCentreHzParam = apvts.getRawParameterValue("TILT_CENTRE_HZ");
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

    // Update multiband transient shaper
    if (lowAttackParam && lowSustainParam &&
        midAttackParam && midSustainParam &&
        highAttackParam && highSustainParam &&
        lowMidXoverParam && midHighXoverParam)
    {
        currentLowAttackDB = lowAttackParam->load();
        currentLowSustainDB = lowSustainParam->load();
        currentMidAttackDB = midAttackParam->load();
        currentMidSustainDB = midSustainParam->load();
        currentHighAttackDB = highAttackParam->load();
        currentHighSustainDB = highSustainParam->load();
        currentLowMidXoverHz = lowMidXoverParam->load();
        currentMidHighXoverHz = midHighXoverParam->load();

        for (auto& mbShaper : multibandTransientShaper)
        {
            mbShaper.setCrossovers(currentLowMidXoverHz, currentMidHighXoverHz);
            mbShaper.setParameters(0, currentLowAttackDB, currentLowSustainDB);    // Low band
            mbShaper.setParameters(1, currentMidAttackDB, currentMidSustainDB);    // Mid band
            mbShaper.setParameters(2, currentHighAttackDB, currentHighSustainDB);  // High band
        }
    }

    auto* transSmoothingParam = apvts.getRawParameterValue("TRANS_SMOOTHING");
    if (transSmoothingParam)
    {
        int smoothingIndex = (int)transSmoothingParam->load();
        TransientShaper::SmoothingMode newMode;

        switch (smoothingIndex)
        {
            case 0: newMode = TransientShaper::SmoothingMode::Sharp; break;
            case 1: newMode = TransientShaper::SmoothingMode::Medium; break;
            case 2: newMode = TransientShaper::SmoothingMode::Smooth; break;
            default: newMode = TransientShaper::SmoothingMode::Medium; break;
        }

        if (newMode != transientSmoothingMode)
        {
            transientSmoothingMode = newMode;
            for (auto& mbShaper : multibandTransientShaper)
                mbShaper.setSmoothingMode(newMode);
        }
    }

    currentTiltCentreHz        = tiltCentreHzParam->load();
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

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TONALITY_HZ", "Tonality Hz",
        juce::NormalisableRange<float>(200.0f, 5000.0f, 1.0f), 5000.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FORMANT_BASE_HZ", "Formant Base Hz",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f), 0.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Multiband Transient Shaper - Low Band
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "LOW_TRANS_ATTACK_DB", "Low Band Attack dB",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.01f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "LOW_TRANS_SUSTAIN_DB", "Low Band Sustain dB",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.01f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Multiband Transient Shaper - Mid Band
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "MID_TRANS_ATTACK_DB", "Mid Band Attack dB",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.01f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "MID_TRANS_SUSTAIN_DB", "Mid Band Sustain dB",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.01f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Multiband Transient Shaper - High Band
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "HIGH_TRANS_ATTACK_DB", "High Band Attack dB",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.01f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "HIGH_TRANS_SUSTAIN_DB", "High Band Sustain dB",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.01f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Multiband Transient Shaper - Crossover Frequencies (using skew for logarithmic feel)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TRANS_LOWMID_XOVER_HZ", "Low/Mid Crossover Hz",
        juce::NormalisableRange<float>(80.0f, 1000.0f, 1.0f, 0.3f), 250.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TRANS_MIDHIGH_XOVER_HZ", "Mid/High Crossover Hz",
        juce::NormalisableRange<float>(1000.0f, 10000.0f, 1.0f, 0.3f), 4000.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Multiband Transient Shaper - Global Smoothing Mode
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        "TRANS_SMOOTHING", "Transient Smoothing",
        juce::StringArray{"Sharp", "Medium", "Smooth"},
        2));


    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        "TILT_CENTRE_HZ", "Tilt Centre Hz", 20, 20000, 2000, "Hz", valueToTextFunction, textToValueFunction));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "TILT_GAIN_DB", "Tilt Gain db",
        juce::NormalisableRange<float>(-6.0f, 6.0f, 0.001f), 0.0f, "dB",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectralShiftAudioProcessor();
}

