#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Constants.h"
#include "MoogKnobLookAndFeel.h"
#include "PluginProcessor.h"

namespace audio_plugin {

class NotchedSlider : public juce::Slider {
public:
    double snapValue(double attemptedValue, DragMode) override {
        if (std::abs(attemptedValue) <= static_cast<double>(kUnityDeadZoneDb))
            return 0.0;
        return attemptedValue;
    }
};

class BoostSelectorSlider : public juce::Slider {
public:
    BoostSelectorSlider() {
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        setRange(0.0, 2.0, 1.0);
        setNumDecimalPlacesToDisplay(0);
        setMouseDragSensitivity(140);
        setScrollWheelEnabled(false);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(3.0f);

        const float titleHeight = juce::jlimit(20.0f, 30.0f, bounds.getHeight() * 0.26f);
        auto selectorArea = bounds;
        selectorArea.removeFromTop(titleHeight);
        selectorArea = selectorArea.reduced(0.0f, 2.0f);

        const float labelWidth = juce::jlimit(26.0f, 34.0f, selectorArea.getWidth() * 0.35f);
        auto valueLabelArea = selectorArea.removeFromLeft(labelWidth);
        auto trackArea = selectorArea.reduced(2.0f, 0.0f);
        const float trackWidth = juce::jlimit(28.0f, 36.0f, trackArea.getWidth() * 0.82f);
        trackArea = trackArea.withSizeKeepingCentre(trackWidth, trackArea.getHeight());

        const float titleWidth = juce::jlimit(42.0f, bounds.getWidth(), bounds.getWidth() * 0.9f);
        const float titleX = juce::jlimit(bounds.getX(), bounds.getRight() - titleWidth,
                                          trackArea.getCentreX() - (titleWidth * 0.5f));
        auto titleArea = juce::Rectangle<float>(titleX, bounds.getY(), titleWidth, titleHeight);
        g.setColour(juce::Colour(0xffe5e5e5));
        const float titleFontSize = juce::jlimit(11.0f, 15.0f, titleHeight * 0.47f);
        g.setFont(titleFontSize);
        g.setFont(g.getCurrentFont().boldened());
        auto titleTop = titleArea.removeFromTop(titleArea.getHeight() * 0.5f);
        auto titleBottom = titleArea;
        g.drawText("BOOST", titleTop.toNearestInt(), juce::Justification::centred, false);
        g.drawText("LEVEL", titleBottom.toNearestInt(), juce::Justification::centred, false);

        auto outerTrack = trackArea.reduced(3.0f, 4.0f);
        g.setColour(juce::Colour(0xff1d1f22));
        g.fillRoundedRectangle(outerTrack, outerTrack.getWidth() * 0.5f);
        g.setColour(juce::Colour(0xff55585c));
        g.drawRoundedRectangle(outerTrack, outerTrack.getWidth() * 0.5f, 1.8f);

        auto innerTrack = outerTrack.reduced(3.8f, 5.2f);
        g.setColour(juce::Colour(0xff0b0d10));
        g.fillRoundedRectangle(innerTrack, innerTrack.getWidth() * 0.48f);

        const float minValue = static_cast<float>(getMinimum());
        const float maxValue = static_cast<float>(getMaximum());
        const float thumbHeight = juce::jlimit(18.0f, 28.0f, innerTrack.getHeight() / 2.5f);
        const float travelTop = innerTrack.getY() + thumbHeight * 0.5f;
        const float travelBottom = innerTrack.getBottom() - thumbHeight * 0.5f;

        auto valueToY = [minValue, maxValue, travelBottom, travelTop](float value) {
            const float normalised = juce::jlimit(0.0f, 1.0f,
                                                  (value - minValue) / (maxValue - minValue));
            return juce::jmap(normalised, 0.0f, 1.0f, travelBottom, travelTop);
        };

        g.setColour(juce::Colour(0xffd7d8db));
        g.setFont(juce::jlimit(12.0f, 17.0f, selectorArea.getHeight() * 0.18f));
        g.setFont(g.getCurrentFont().boldened());
        auto drawValueLabel = [&](const juce::String& text, float value) {
            const int x = juce::roundToInt(valueLabelArea.getX());
            const int labelHeight = juce::roundToInt(juce::jlimit(18.0f, 24.0f,
                                                                   selectorArea.getHeight() * 0.2f));
            const int y = juce::roundToInt(valueToY(value) - (static_cast<float>(labelHeight) * 0.5f));
            const int width = juce::roundToInt(valueLabelArea.getWidth());
            g.drawFittedText(text, x, y, width, labelHeight, juce::Justification::centredRight,
                             1);
        };

        drawValueLabel("+12", 2.0f);
        drawValueLabel("+6", 1.0f);
        drawValueLabel("0", 0.0f);

        const float selectedY = valueToY(static_cast<float>(getValue()));
        const float thumbWidth = juce::jlimit(14.0f, innerTrack.getWidth() - 2.0f,
                                              innerTrack.getWidth() * 0.78f);
        auto thumbBounds = juce::Rectangle<float>(
            innerTrack.getCentreX() - (thumbWidth * 0.5f), selectedY - (thumbHeight * 0.5f),
            thumbWidth, thumbHeight);

        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff6f7378), thumbBounds.getCentreX(),
                                               thumbBounds.getY(), juce::Colour(0xff2e3237),
                                               thumbBounds.getCentreX(), thumbBounds.getBottom(),
                                               false));
        const float thumbRadius =
            juce::jmin(thumbBounds.getWidth(), thumbBounds.getHeight()) * 0.48f;
        g.fillRoundedRectangle(thumbBounds, thumbRadius);
        g.setColour(juce::Colour(0xff9a9da2));
        g.drawRoundedRectangle(thumbBounds, thumbRadius, 1.0f);

        g.setColour(juce::Colour(0x88000000));
        auto notch = thumbBounds.withSizeKeepingCentre(thumbBounds.getWidth() * 0.42f, 2.8f);
        g.fillRoundedRectangle(notch, 1.5f);
    }
};

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef_;
    MoogKnobLookAndFeel moogLookAndFeel_;

    NotchedSlider lowSlider_;
    NotchedSlider midSlider_;
    NotchedSlider highSlider_;
    BoostSelectorSlider boostSlider_;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> boostAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

}  // namespace audio_plugin
