//
// Created by Kyle Ramsey on 15/12/2025.
//

#include "XYPad.h"

// ===== Thumb Implementation =====

XYPad::Thumb::Thumb(XYPad& parent) : parentPad(parent)
{
	constrainer.setMinimumOnscreenAmounts(thumbSize, thumbSize, thumbSize, thumbSize);
}

juce::Point<float> XYPad::Thumb::getNormalisedPosition() const
{
	auto parentBounds = getParentComponent()->getLocalBounds().toFloat();
	float normalizedX = getX() / (parentBounds.getWidth() - thumbSize);
	float normalizedY = 1.0f - (getY() / (parentBounds.getHeight() - thumbSize)); // Invert Y
	return {normalizedX, normalizedY};
}

juce::Colour XYPad::Thumb::calculateThumbColour() const
{
	auto pos = getNormalisedPosition();

	// Interpolate on X axis (pitch)
	auto pitchColour = parentPad.pitchNegativeColour.interpolatedWith(
		parentPad.pitchPositiveColour, pos.x);

	// Interpolate on Y axis (formant)
	auto formantColour = parentPad.formantNegativeColour.interpolatedWith(
		parentPad.formantPositiveColour, pos.y);

	// Blend the two interpolated colors
	return pitchColour.interpolatedWith(formantColour, parentPad.colorBlendRatio);
}

void XYPad::Thumb::paint(juce::Graphics& g)
{
	auto colour = calculateThumbColour();

	// Darken when dragging
	if (isDragging)
		colour = colour.darker(parentPad.darkeningAmount);

	g.setColour(colour);
	g.fillEllipse(getLocalBounds().toFloat());
}

void XYPad::Thumb::mouseDown(const juce::MouseEvent& event)
{
	isDragging = true;
	repaint();
	dragger.startDraggingComponent(this, event);
}

void XYPad::Thumb::mouseUp(const juce::MouseEvent& event)
{
	isDragging = false;
	repaint();
}

void XYPad::Thumb::mouseDrag(const juce::MouseEvent& event)
{
	dragger.dragComponent(this, event, &constrainer);
	if (moveCallback)
		moveCallback(getPosition().toDouble());
}

void XYPad::Thumb::mouseDoubleClick(const juce::MouseEvent& event)
{
	if (resetCallback)
		resetCallback();
}

// ===== XYPad Implementation =====

XYPad::XYPad() : thumb(*this)
{
	addAndMakeVisible(thumb);

	// Setup move callback
	thumb.moveCallback = [this](juce::Point<double> position)
	{
		updateSlidersFromThumbPosition(position);
	};

	// Setup reset callback
	thumb.resetCallback = [this]()
	{
		resetThumbToDefaultPosition();
	};
}

void XYPad::resized()
{
	thumb.setBounds(getLocalBounds().withSizeKeepingCentre(thumbSize, thumbSize));
	if (!xSliders.empty())
		sliderValueChanged(xSliders[0]);
	if (!ySliders.empty())
		sliderValueChanged(ySliders[0]);
}

void XYPad::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();

	// Fill background
	g.setColour(backgroundColour);
	g.fillRoundedRectangle(bounds, cornerRadius);

	// Draw border
	g.setColour(backgroundColour.brighter(0.3f));  // Lighter border
	g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);  // 2 px border thickness
}

void XYPad::registerSlider(juce::Slider* slider, Axis axis)
{
	slider->addListener(this);
	const std::lock_guard<std::mutex> lock(vectorMutex);
	if (axis == Axis::X)
		xSliders.push_back(slider);
	if (axis == Axis::Y)
		ySliders.push_back(slider);
}

void XYPad::deregisterSlider(juce::Slider* slider)
{
	slider->removeListener(this);
	const std::lock_guard<std::mutex> lock(vectorMutex);
	// remove/erase idiom
	std::erase(xSliders, slider);
	std::erase(ySliders, slider);
}

void XYPad::setThumbColours(juce::Colour pitchPos, juce::Colour pitchNeg,
                            juce::Colour formantPos, juce::Colour formantNeg)
{
	pitchPositiveColour = pitchPos;
	pitchNegativeColour = pitchNeg;
	formantPositiveColour = formantPos;
	formantNegativeColour = formantNeg;
	repaint();
}

void XYPad::setBackgroundColour(juce::Colour colour)
{
	backgroundColour = colour;
	repaint();
}

void XYPad::sliderValueChanged(juce::Slider* slider)
{
	// Avoid loopback
	if (thumb.isMouseOverOrDragging(false))
		return;

	// Figure out if the slider belongs to xSliders or ySliders
	const auto isXAxisSlider = std::ranges::find(xSliders, slider) != xSliders.end();
	const auto bounds = getLocalBounds().toDouble();
	constexpr auto w = static_cast<double>(thumbSize);

	if (isXAxisSlider)
	{
		thumb.setTopLeftPosition(
			juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w),
			thumb.getY());
	}
	else
	{
		thumb.setTopLeftPosition(
			thumb.getX(),
			juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w, 0.0));
	}
	repaint();
}

void XYPad::updateSlidersFromThumbPosition(juce::Point<double> thumbPos) const {
	const auto bounds = getLocalBounds().toDouble();
	constexpr auto w = static_cast<double>(thumbSize);

	// Update X-axis sliders
	for (auto* slider : xSliders)
	{
		slider->setValue(juce::jmap(thumbPos.getX(), bounds.getHeight() - w, 0.0,
		                            slider->getMaximum(), slider->getMinimum()));
	}

	// Update Y-axis sliders
	for (auto* slider : ySliders)
	{
		slider->setValue(juce::jmap(thumbPos.getY(), bounds.getHeight() - w, 0.0,
		                            slider->getMinimum(), slider->getMaximum()));
	}
}

void XYPad::resetThumbToDefaultPosition()
{
	// Reset all X sliders to their double-click return value
	for (auto* slider : xSliders)
	{
		slider->setValue(slider->getDoubleClickReturnValue());
	}

	// Reset all Y sliders to their double-click return value
	for (auto* slider : ySliders)
	{
		slider->setValue(slider->getDoubleClickReturnValue());
	}

	// Update thumb position to match reset values
	auto thumbPosition = calculateThumbPositionFromSliders();
	thumb.setTopLeftPosition(thumbPosition);
	repaint();
}

juce::Point<int> XYPad::calculateThumbPositionFromSliders() const
{
	const auto bounds = getLocalBounds().toDouble();
	constexpr auto w = static_cast<double>(thumbSize);

	double thumbX = thumb.getX();
	double thumbY = thumb.getY();

	// Calculate X position from first X slider
	if (!xSliders.empty())
	{
		auto* slider = xSliders[0];
		thumbX = juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(),
		                    0.0, bounds.getWidth() - w);
	}

	// Calculate Y position from first Y slider
	if (!ySliders.empty())
	{
		auto* slider = ySliders[0];
		thumbY = juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(),
		                    bounds.getHeight() - w, 0.0);
	}

	return {static_cast<int>(thumbX), static_cast<int>(thumbY)};
}