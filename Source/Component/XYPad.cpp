//
// Created by Kyle Ramsey on 15/12/2025.
//

#include "XYPad.h"

XYPad::Thumb::Thumb()
	{
		constrainer.setMinimumOnscreenAmounts(thumbSize, thumbSize, thumbSize, thumbSize);
	}

void XYPad::Thumb::paint(juce::Graphics& g)
	{
		g.setColour(juce::Colours::white);
		g.fillEllipse(getLocalBounds().toFloat());
	}

	void XYPad::Thumb::mouseDown(const juce::MouseEvent& event)
	{
		dragger.startDraggingComponent(this, event);
	}

	void XYPad::Thumb::mouseDrag(const juce::MouseEvent& event)
	{
		dragger.dragComponent(this, event, &constrainer);
		if (moveCallback)
			moveCallback(getPosition().toDouble());
	}

	/*
	 * XY Pad section
	 */
	XYPad::XYPad()
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
		g.setColour(juce::Colours::black);
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
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