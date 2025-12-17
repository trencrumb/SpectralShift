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
        Thumb();
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;
        std::function<void(juce::Point<double>)> moveCallback;
        std::function<void()> resetCallback;
    private:
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        bool isDragging = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb)
    };

    XYPad();
    void resized() override;
    void paint(juce::Graphics& g) override;
    void registerSlider(juce::Slider* slider, Axis axis);
    void deregisterSlider(juce::Slider* slider);
    private:
    void sliderValueChanged(juce::Slider* slider) override;

    std::vector<juce::Slider*> xSliders, ySliders;
    Thumb thumb;
    std::mutex vectorMutex;

    static constexpr int thumbSize = 40;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPad)

};