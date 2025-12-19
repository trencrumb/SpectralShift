//
// Created by Kyle Ramsey on 16/12/2025.
//

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Color palette
    struct Colors
    {
        inline static const juce::Colour background = juce::Colour(0xFF0F1220);
        inline static const juce::Colour backgroundLight = juce::Colour(0xff2a2a2a);
        inline static const juce::Colour backgroundDark = juce::Colour(0xff0f0f0f);

        inline static const juce::Colour primary = juce::Colour(0xFF3A7BFF);
        inline static const juce::Colour primaryLight = juce::Colour(0xff4FC3F7);
        inline static const juce::Colour primaryDark = juce::Colour(0xff0077cc);

        inline static const juce::Colour secondary = juce::Colour(0xffFF6B9D);
        inline static const juce::Colour accent = juce::Colour(0xffFFC107);

        inline static const juce::Colour text = juce::Colour(0xFFE6E9FF);
        inline static const juce::Colour textDim = juce::Colour(0xFF9AA1C4);

        inline static const juce::Colour pitchPositive = juce::Colour(0xFF3A7BFF);
        inline static const juce::Colour pitchNegative = juce::Colour(0xFF0B1833);

        inline static const juce::Colour formantPositive = juce::Colour(0xFF9B6BFF);
        inline static const juce::Colour formantNegative = juce::Colour(0xFF25193D);

        inline static const juce::Colour formantCompensation = juce::Colour(0xFF5a7afa);

        inline static const juce::Colour tonality = juce::Colour(0xFF4576f5);
        inline static const juce::Colour tilt = juce::Colour(0xFFFFB454);

        inline static const juce::Colour error = juce::Colour(0xFFE57373);
        inline static const juce::Colour transparent = juce::Colour(0x00121212);

        inline static const juce::Colour xyPadBackground = juce::Colour(0xFF222329);
        inline static const juce::Colour xyPadThumb = juce::Colour(0xFFE6E9FF);
        inline static const juce::Colour xyPadThumbDragging = juce::Colour(0xFFB0B3CC);


    };

    CustomLookAndFeel()
    {
        // Set default colors for components
        setColour(juce::Slider::thumbColourId, Colors::primary);
        setColour(juce::Slider::trackColourId, Colors::primary);
        setColour(juce::Slider::backgroundColourId, Colors::backgroundDark);
        setColour(juce::Slider::textBoxTextColourId, Colors::text);
        setColour(juce::Slider::textBoxBackgroundColourId, Colors::backgroundDark);
        setColour(juce::Slider::textBoxOutlineColourId, Colors::transparent);

        setColour(juce::Label::textColourId, Colors::text);
        setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        setColour(juce::TextButton::buttonColourId, Colors::backgroundLight);
        setColour(juce::TextButton::buttonOnColourId, Colors::primary);
        setColour(juce::TextButton::textColourOffId, Colors::text);
        setColour(juce::TextButton::textColourOnId, Colors::text);

        setColour(juce::ComboBox::backgroundColourId, Colors::backgroundDark);
        setColour(juce::ComboBox::textColourId, Colors::text);
        setColour(juce::ComboBox::outlineColourId, Colors::backgroundLight);
        setColour(juce::ComboBox::arrowColourId, Colors::primary);
    }

    // Custom slider drawing
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackColour = slider.findColour(juce::Slider::trackColourId);  // <-- Move to top


        if (style == juce::Slider::LinearVertical)
        {
            float trackWidth = 4.0f;
            float trackX = x + width * 0.5f - trackWidth * 0.5f;

            // Check if this is a bipolar cents slider
            if (slider.getName() == "CentsSlider")
            {
                // Draw bipolar vertical slider (center at middle)
                float centerY = y + height * 0.5f;

                // Background track
                g.setColour(Colors::backgroundDark);
                g.fillRoundedRectangle(trackX, y, trackWidth, height, trackWidth * 0.5f);

                // Fill from center to thumb position
                float trackHeight = std::abs(sliderPos - centerY);
                float trackY = std::min(sliderPos, centerY);

                g.setColour(trackColour);
                g.fillRoundedRectangle(trackX, trackY, trackWidth, trackHeight, trackWidth * 0.5f);

                // Thumb
                float thumbSize = 16.0f;
                float thumbX = x + width * 0.5f - thumbSize * 0.5f;
                float thumbY = sliderPos - thumbSize * 0.5f;

                g.setColour(trackColour.brighter(0.3f));
                g.fillEllipse(thumbX, thumbY, thumbSize, thumbSize);

                g.setColour(trackColour);
                g.fillEllipse(thumbX + 3, thumbY + 3, thumbSize - 6, thumbSize - 6);
            }
            else
            {
                // Default unipolar vertical slider (fill from bottom)
                g.setColour(Colors::backgroundDark);
                g.fillRoundedRectangle(trackX, y, trackWidth, height, trackWidth * 0.5f);

                // Track fill from bottom to thumb
                float trackHeight = sliderPos - y;
                g.setColour(trackColour);
                g.fillRoundedRectangle(trackX, sliderPos, trackWidth, height - trackHeight, trackWidth * 0.5f);

                // Thumb
                float thumbSize = 16.0f;
                float thumbX = x + width * 0.5f - thumbSize * 0.5f;
                float thumbY = sliderPos - thumbSize * 0.5f;

                g.setColour(trackColour.brighter(0.3f));
                g.fillEllipse(thumbX, thumbY, thumbSize, thumbSize);

                g.setColour(trackColour);
                g.fillEllipse(thumbX + 3, thumbY + 3, thumbSize - 6, thumbSize - 6);
            }
        }

        else if (style == juce::Slider::LinearHorizontal)
        {
            auto trackColour = slider.findColour(juce::Slider::trackColourId);

            // Check if this is the tonality slider
            if (slider.getName() == "TonalitySlider")
            {
                // Draw tonality slider differently (e.g., no center, full fill from left)
                float trackHeight = 4.0f;
                float trackY = y + height * 0.5f - trackHeight * 0.5f;

                // Background
                g.setColour(Colors::backgroundDark);
                g.fillRoundedRectangle(x, trackY, width, trackHeight, trackHeight * 0.5f);

                // Fill from left to thumb position
                g.setColour(trackColour);
                g.fillRoundedRectangle(x, trackY, sliderPos - x, trackHeight, trackHeight * 0.5f);

                // Thumb
                float thumbSize = 16.0f;
                float thumbX = sliderPos - thumbSize * 0.5f;
                float thumbY = y + height * 0.5f - thumbSize * 0.5f;

                g.setColour(trackColour.brighter(0.3f));
                g.fillEllipse(thumbX, thumbY, thumbSize, thumbSize);

                g.setColour(trackColour);
                g.fillEllipse(thumbX + 3, thumbY + 3, thumbSize - 6, thumbSize - 6);
            }
            else
            {
                // Default horizontal slider (tilt, etc.) - center-based
                float trackHeight = 4.0f;
                float trackY = y + height * 0.5f - trackHeight * 0.5f;
                g.setColour(Colors::backgroundDark);
                g.fillRoundedRectangle(x, trackY, width, trackHeight, trackHeight * 0.5f);

                // Track fill (from center)
                float centerX = x + width * 0.5f;
                float trackWidth = std::abs(sliderPos - centerX);
                float trackX = std::min(sliderPos, centerX);

                g.setColour(trackColour);
                g.fillRoundedRectangle(trackX, trackY, trackWidth, trackHeight, trackHeight * 0.5f);

                // Thumb
                float thumbSize = 16.0f;
                float thumbX = sliderPos - thumbSize * 0.5f;
                float thumbY = y + height * 0.5f - thumbSize * 0.5f;

                g.setColour(trackColour.brighter(0.3f));
                g.fillEllipse(thumbX, thumbY, thumbSize, thumbSize);

                g.setColour(trackColour);
                g.fillEllipse(thumbX + 3, thumbY + 3, thumbSize - 6, thumbSize - 6);
            }
        }
        else
        {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                                   minSliderPos, maxSliderPos, style, slider);
        }

    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Make a square toggle box (20x20 pixels for better visibility)
        auto toggleSize = 20.0f;
        auto toggleBounds = juce::Rectangle<float>(0, 0, toggleSize, toggleSize)
            .withCentre(bounds.getCentre());

        // Draw square toggle box background
        g.setColour(Colors::backgroundDark);
        g.fillRect(toggleBounds);

        // Fill if toggled on
        if (button.getToggleState())
        {
            if (button.getName() == "Formant") {
                g.setColour(Colors::formantPositive);
            } else if (button.getName() == "Tilt") {
                g.setColour(Colors::tilt);
            }
            auto fill = toggleBounds.reduced(4.0f);
            g.fillRect(fill);
        }

        // Draw border (thicker for visibility)
        g.setColour(Colors::backgroundLight);
        g.drawRect(toggleBounds, 2.0f);
    }


    // Custom rotary slider drawing
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        // Background arc
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                   arcRadius, arcRadius, 0.0f,
                                   rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colors::backgroundDark);
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value arc
        if (toAngle > rotaryStartAngle)
        {
            juce::Path valueArc;
            valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                  arcRadius, arcRadius, 0.0f,
                                  rotaryStartAngle, toAngle, true);
            auto trackColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
            g.setColour(trackColour);
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Pointer
        juce::Path pointer;
        auto pointerLength = radius * 0.5f;
        auto pointerThickness = lineW * 0.6f;
        pointer.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(bounds.getCentreX(), bounds.getCentreY()));

        auto trackColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(trackColour.brighter(0.5f));
        g.fillPath(pointer);
    }

    // Custom button drawing
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
        auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                                         .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

        g.setColour(baseColour);

        auto cornerSize = 4.0f;
        if (button.isConnectedOnLeft() || button.isConnectedOnRight())
        {
            juce::Path path;
            path.addRoundedRectangle(bounds.getX(), bounds.getY(),
                                    bounds.getWidth(), bounds.getHeight(),
                                    cornerSize, cornerSize,
                                    !button.isConnectedOnLeft(),
                                    !button.isConnectedOnRight(),
                                    !button.isConnectedOnLeft(),
                                    !button.isConnectedOnRight());
            g.fillPath(path);

            g.setColour(button.findColour(juce::ComboBox::outlineColourId));
            g.strokePath(path, juce::PathStrokeType(1.0f));
        }
        else
        {
            g.fillRoundedRectangle(bounds, cornerSize);

            g.setColour(button.findColour(juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
    }
};