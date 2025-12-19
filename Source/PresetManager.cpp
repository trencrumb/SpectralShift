//
// Created by Kyle Ramsey on 19/12/2025.
//

#include "PresetManager.h"

PresetManager::PresetManager()
{
    initializeFactoryPresets();
    numFactoryPresets = static_cast<int>(presets.size());
}

juce::String PresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return {};

    return presets[static_cast<size_t>(index)].name;
}

const PresetManager::Preset* PresetManager::getPreset(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return nullptr;

    return &presets[static_cast<size_t>(index)];
}

bool PresetManager::applyPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return false;

    const auto& preset = presets[static_cast<size_t>(index)];

    // Apply all parameter values from the preset
    for (const auto& [paramID, value] : preset.values)
    {
        if (auto* param = apvts.getParameter(paramID))
        {
            const auto normalizedValue = param->getNormalisableRange().convertTo0to1(value);
            param->setValueNotifyingHost(normalizedValue);
        }
    }

    currentPresetIndex = index;
    return true;
}

void PresetManager::savePreset(const juce::String& name, const juce::AudioProcessorValueTreeState& apvts)
{
    Preset newPreset;
    newPreset.name = name;

    // Get all parameters from APVTS
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            const auto paramID = rangedParam->getParameterID();
            const auto value = rangedParam->getNormalisableRange().convertFrom0to1(rangedParam->getValue());
            newPreset.values[paramID] = value;
        }
    }

    presets.push_back(newPreset);
    currentPresetIndex = static_cast<int>(presets.size()) - 1;
}

void PresetManager::renamePreset(int index, const juce::String& newName)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;

    presets[static_cast<size_t>(index)].name = newName;
}

bool PresetManager::deletePreset(int index)
{
    // Cannot delete factory presets
    if (index < 0 || index >= static_cast<int>(presets.size()) || index < numFactoryPresets)
        return false;

    presets.erase(presets.begin() + index);

    // Update current preset index if needed
    if (currentPresetIndex == index)
        currentPresetIndex = -1;
    else if (currentPresetIndex > index)
        currentPresetIndex--;

    return true;
}

void PresetManager::resetToFactoryPresets()
{
    presets.clear();
    initializeFactoryPresets();
    numFactoryPresets = static_cast<int>(presets.size());
    currentPresetIndex = -1;
}

void PresetManager::initializeFactoryPresets()
{
    // Factory Preset 1: Subtle Brighten
    presets.push_back({
        "Subtle Brighten",
        {
            { "PITCH_SEMITONES",        2.0f },
            { "PITCH_CENTS",            0.0f },
            { "FORMANT_SEMITONES",      1.0f },
            { "FORMANT_CENTS",          0.0f },
            { "FORMANT_COMPENSATION",   1.0f },
            { "TONALITY_HZ",         6000.0f },
            { "FORMANT_BASE_HZ",      200.0f },
            { "TILT_GAIN_DB",           1.5f },
            { "TILT_CENTRE_HZ",      1000.0f },
            { "TILT_CENTRE_AUTO",       1.0f }
        }
    });

    // Factory Preset 2: Thick Low
    presets.push_back({
        "Thick Low",
        {
            { "PITCH_SEMITONES",        7.0f },
            { "PITCH_CENTS",            0.0f },
            { "FORMANT_SEMITONES",     -5.0f },
            { "FORMANT_CENTS",          0.0f },
            { "FORMANT_COMPENSATION",   1.0f },
            { "TONALITY_HZ",         3000.0f },
            { "FORMANT_BASE_HZ",      150.0f },
            { "TILT_GAIN_DB",          -2.0f },
            { "TILT_CENTRE_HZ",      1000.0f },
            { "TILT_CENTRE_AUTO",       1.0f }
        }
    });

    // Factory Preset 3: Default/Init
    presets.push_back({
        "Default",
        {
            { "PITCH_SEMITONES",        0.0f },
            { "PITCH_CENTS",            0.0f },
            { "FORMANT_SEMITONES",      0.0f },
            { "FORMANT_CENTS",          0.0f },
            { "FORMANT_COMPENSATION",   1.0f },
            { "TONALITY_HZ",         5000.0f },
            { "FORMANT_BASE_HZ",        0.0f },
            { "TILT_GAIN_DB",           0.0f },
            { "TILT_CENTRE_HZ",      1000.0f },
            { "TILT_CENTRE_AUTO",       1.0f }
        }
    });
}
