//
// Created by Kyle Ramsey on 15/12/2025.
//
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * XY Pad component that allows controlling two parameters simultaneously.
 * The thumb color interpolates between pitch and formant colors based on position.
 */
class XYPad : public juce::Component, juce::Slider::Listener
{
public:
    enum class Axis { X, Y };

    // ===== Thumb Component =====
    /**
     * Draggable thumb component with dynamic color based on position.
     */
    class Thumb : public Component
    {
    public:
        Thumb(XYPad& parent);

        // ===== Component Overrides =====
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;

        // ===== Callbacks =====
        std::function<void(juce::Point<double>)> moveCallback;
        std::function<void()> resetCallback;

    private:
        // ===== Helper Methods =====
        /** Returns normalized position (0-1) of thumb within parent bounds. */
        juce::Point<float> getNormalisedPosition() const;

        /** Calculates interpolated color based on normalized thumb position. */
        juce::Colour calculateThumbColour() const;

        // ===== Members =====
        XYPad& parentPad;
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        bool isDragging = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb)
    };

    // ===== Construction =====
    XYPad();

    // ===== Component Overrides =====
    void resized() override;
    void paint(juce::Graphics& g) override;

    // ===== Slider Registration =====
    /** Registers a slider to control the XY pad on the specified axis. */
    void registerSlider(juce::Slider* slider, Axis axis);

    /** Deregisters a slider from the XY pad. */
    void deregisterSlider(juce::Slider* slider);

    // ===== Appearance =====
    /** Sets the interpolation colors for the thumb based on position. */
    void setThumbColours(juce::Colour pitchPos, juce::Colour pitchNeg,
                         juce::Colour formantPos, juce::Colour formantNeg);

    /** Sets the background color of the XY pad. */
    void setBackgroundColour(juce::Colour colour);

private:
    // ===== Slider Listener =====
    void sliderValueChanged(juce::Slider* slider) override;

    // ===== Helper Methods =====
    /** Updates registered sliders based on thumb position. */
    void updateSlidersFromThumbPosition(juce::Point<double> thumbPos) const;

    /** Calculates thumb position from registered slider values. */
    juce::Point<int> calculateThumbPositionFromSliders() const;

    /** Resets thumb to default position based on slider double-click values. */
    void resetThumbToDefaultPosition();

    // ===== Members =====
    std::vector<juce::Slider*> xSliders, ySliders;
    Thumb thumb;
    std::mutex vectorMutex;

    // ===== Constants =====
    static constexpr int thumbSize = 40;
    static constexpr float cornerRadius = 20.0f;
    static constexpr float darkeningAmount = 0.3f;
    static constexpr float colorBlendRatio = 0.5f;

    // ===== Colors =====
    juce::Colour backgroundColour{0xFF222329};
    juce::Colour pitchPositiveColour{0xFF3A7BFF};
    juce::Colour pitchNegativeColour{0xFF0B1833};
    juce::Colour formantPositiveColour{0xFF9B6BFF};
    juce::Colour formantNegativeColour{0xFF25193D};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPad)
};