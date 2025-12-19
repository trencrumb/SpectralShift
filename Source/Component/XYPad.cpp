//
// Created by Kyle Ramsey on 15/12/2025.
//

#include "XYPad.h"

XYPad::Thumb::Thumb(XYPad& parent) : parentPad(parent)
	{
		constrainer.setMinimumOnscreenAmounts(thumbSize, thumbSize, thumbSize, thumbSize);
	}

void XYPad::Thumb::paint(juce::Graphics& g)
{
	// Get normalized position (0-1 range)
	auto parentBounds = getParentComponent()->getLocalBounds().toFloat();
	float normalizedX = getX() / (parentBounds.getWidth() - thumbSize);
	float normalizedY = 1.0f - (getY() / (parentBounds.getHeight() - thumbSize)); // Invert Y

	// Interpolate on X axis
	auto xColour = parentPad.xNegativeColour.interpolatedWith(
		parentPad.xPositiveColour, normalizedX);

	// Interpolate on Y axis
	auto yColour = parentPad.yNegativeColour.interpolatedWith(
		parentPad.yPositiveColour, normalizedY);

	// Blend the two interpolated colors
	auto finalColour = xColour.interpolatedWith(yColour, 0.5f);

	// Darken when dragging
	if (isDragging)
		finalColour = finalColour.darker(0.3f);  // 30% darker

	g.setColour(finalColour);
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

void XYPad::Thumb::setThumbColour(juce::Colour normal, juce::Colour dragging) {
	thumbColour = normal;
	thumbDraggingColour = dragging;
}

	/*
	 * XY Pad section
	 */
	XYPad::XYPad() : thumb(*this)
	{
		addAndMakeVisible(thumb);
		thumb.moveCallback = [&](juce::Point<double> position)
		{
				const auto bounds = getLocalBounds().toDouble();
				const auto w = static_cast<double>(thumbSize);


			for (auto* slider : xSliders)
			{
				slider->setValue(juce::jmap(position.getX(), bounds.getHeight() - w, 0.0, slider->getMaximum(), slider->getMinimum()));
			}

			for (auto* slider : ySliders)
			{
				slider->setValue(juce::jmap(position.getY(), bounds.getHeight() - w, 0.0, slider->getMinimum(), slider->getMaximum()));
			}
		};

		thumb.resetCallback = [&]()
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
			const auto bounds = getLocalBounds().toDouble();
			const auto w = static_cast<double>(thumbSize);

			double thumbX = thumb.getX();
			double thumbY = thumb.getY();

			// Calculate X position from first X slider
			if (!xSliders.empty())
			{
				auto* slider = xSliders[0];
				thumbX = juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w);
			}

			// Calculate Y position from first Y slider
			if (!ySliders.empty())
			{
				auto* slider = ySliders[0];
				thumbY = juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w, 0.0);
			}

			thumb.setTopLeftPosition(static_cast<int>(thumbX), static_cast<int>(thumbY));
			repaint();
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
		g.setColour(backgroundColour);
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 20.f);
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
		xSliders.erase(std::remove(xSliders.begin(), xSliders.end(), slider), xSliders.end());
		ySliders.erase(std::remove(ySliders.begin(), ySliders.end(), slider), ySliders.end());
	}

	void XYPad::sliderValueChanged(juce::Slider* slider)
	{
		// Avoid loopback
		if (thumb.isMouseOverOrDragging(false))
			return;

		// Figure out if the slider belongs to xSliders or ySliders
		const auto isXAxisSlider = std::find(xSliders.begin(), xSliders.end(), slider) != xSliders.end();
		const auto bounds = getLocalBounds().toDouble();
		const auto w = static_cast<double>(thumbSize);
		if (isXAxisSlider)
		{
			thumb.setTopLeftPosition(
				juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), 0.0, bounds.getWidth() - w),
				thumb.getY());
		} else
		{
			thumb.setTopLeftPosition(
				thumb.getX(),
				juce::jmap(slider->getValue(), slider->getMinimum(), slider->getMaximum(), bounds.getHeight() - w, 0.0));
		}
		repaint();
	}
	void XYPad::setBackgroundColour(juce::Colour colour)
	{
		backgroundColour = colour;
		repaint();
	}