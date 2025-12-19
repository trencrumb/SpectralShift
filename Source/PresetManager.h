//
// Created by Kyle Ramsey on 19/12/2025.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include <vector>

/**
 * Manages audio processor presets.
 *
 * Handles loading, saving, and applying parameter presets for the plugin.
 * Presets are defined as collections of parameter values that can be
 * applied to the AudioProcessorValueTreeState.
 */
class PresetManager
{
public:
    /**
     * Represents a single preset with a name and parameter values.
     */
    struct Preset
    {
        juce::String name;
        std::map<juce::String, float> values;  // paramID -> value
    };

    /**
     * Constructs a PresetManager and initializes factory presets.
     */
    PresetManager();

    // ===== Preset Access =====

    /** Returns the total number of available presets. */
    int getNumPresets() const { return static_cast<int>(presets.size()); }

    /** Returns the index of the currently selected preset (-1 if none). */
    int getCurrentPresetIndex() const { return currentPresetIndex; }

    /** Returns the name of the preset at the given index. */
    juce::String getPresetName(int index) const;

    /** Returns the preset at the given index. */
    const Preset* getPreset(int index) const;

    // ===== Preset Management =====

    /**
     * Applies a preset to the given APVTS.
     * Returns true if successful, false if index is invalid.
     */
    bool applyPreset(int index, juce::AudioProcessorValueTreeState& apvts);

    /**
     * Saves current APVTS state as a new preset.
     */
    void savePreset(const juce::String& name, const juce::AudioProcessorValueTreeState& apvts);

    /**
     * Renames the preset at the given index.
     */
    void renamePreset(int index, const juce::String& newName);

    /**
     * Deletes the preset at the given index (factory presets cannot be deleted).
     */
    bool deletePreset(int index);

    /**
     * Resets to factory presets.
     */
    void resetToFactoryPresets();

private:
    std::vector<Preset> presets;
    int currentPresetIndex = -1;
    int numFactoryPresets = 0;  // Track how many presets are factory (cannot be deleted)

    /** Initializes the factory presets. */
    void initializeFactoryPresets();
};
