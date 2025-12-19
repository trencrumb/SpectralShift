//
// Created by Kyle Ramsey on 15/12/2025.
//
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>


class XYPad : public juce::Component, juce::Slider::Listener {
public:
    enum class Axis { X, Y };

    class Thumb : public Component
    {
    public:
        Thumb(XYPad& parent);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;
        std::function<void(juce::Point<double>)> moveCallback;
        std::function<void()> resetCallback;
        void setThumbColour(juce::Colour normal, juce::Colour dragging);

    private:
        XYPad& parentPad;
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        bool isDragging = false;
        juce::Colour thumbColour{0xFFE6E9FF};
        juce::Colour thumbDraggingColour{0xFFB0B3CC};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb)
    };

    XYPad();
    void resized() override;
    void paint(juce::Graphics& g) override;
    void registerSlider(juce::Slider* slider, Axis axis);
    void deregisterSlider(juce::Slider* slider);

    private:
    void sliderValueChanged(juce::Slider* slider) override;
    void setBackgroundColour(juce::Colour colour);

    std::vector<juce::Slider*> xSliders, ySliders;
    Thumb thumb;
    std::mutex vectorMutex;

    static constexpr int thumbSize = 40;

    juce::Colour backgroundColour{0xFF222329};
    juce::Colour xPositiveColour{0xFF3A7BFF};
    juce::Colour xNegativeColour{0xFF0B1833};
    juce::Colour yPositiveColour{0xFF9B6BFF};
    juce::Colour yNegativeColour{0xFF25193D};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPad)

};