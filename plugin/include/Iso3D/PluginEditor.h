#pragma once

#include <cstddef>

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
    enum ColourIds {
        titleTextColourId = 0x2801000,
        levelLabelTextColourId,
        trackOuterFillColourId,
        trackOuterOutlineColourId,
        trackInnerFillColourId,
        thumbTopColourId,
        thumbBottomColourId,
        thumbOutlineColourId,
        thumbNotchColourId
    };

    BoostSelectorSlider() {
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        setRange(0.0, 2.0, 1.0);
        setNumDecimalPlacesToDisplay(0);
        setMouseDragSensitivity(140);
        setScrollWheelEnabled(false);

        setColour(titleTextColourId, juce::Colour(0xffe5e5e5));
        setColour(levelLabelTextColourId, juce::Colour(0xffd7d8db));
        setColour(trackOuterFillColourId, juce::Colour(0xff1d1f22));
        setColour(trackOuterOutlineColourId, juce::Colour(0xff55585c));
        setColour(trackInnerFillColourId, juce::Colour(0xff0b0d10));
        setColour(thumbTopColourId, juce::Colour(0xff6f7378));
        setColour(thumbBottomColourId, juce::Colour(0xff2e3237));
        setColour(thumbOutlineColourId, juce::Colour(0xff9a9da2));
        setColour(thumbNotchColourId, juce::Colour(0x88000000));
    }

    void paint(juce::Graphics& g) override {
        const auto geometry = calculateGeometry();
        drawTitle(g, geometry);
        drawTrack(g, geometry);
        drawLevelLabels(g, geometry);
        drawThumb(g, geometry);
    }

private:
    struct Geometry {
        juce::Rectangle<float> bounds;
        juce::Rectangle<float> selectorArea;
        juce::Rectangle<float> valueLabelArea;
        juce::Rectangle<float> trackArea;
        juce::Rectangle<float> outerTrack;
        juce::Rectangle<float> innerTrack;
        juce::Rectangle<float> titleTop;
        juce::Rectangle<float> titleBottom;
        float titleHeight = 0.0f;
        float labelHeight = 0.0f;
        float thumbHeight = 0.0f;
    };

    static constexpr std::size_t kNumBoostLevels = sizeof(kBoostLevels) / sizeof(kBoostLevels[0]);

    static constexpr float kBoundsInset = 3.0f;
    static constexpr float kMinTitleHeight = 20.0f;
    static constexpr float kMaxTitleHeight = 30.0f;
    static constexpr float kTitleHeightRatio = 0.26f;
    static constexpr float kSelectorVerticalInset = 2.0f;
    static constexpr float kMinLabelWidth = 26.0f;
    static constexpr float kMaxLabelWidth = 34.0f;
    static constexpr float kLabelWidthRatio = 0.35f;
    static constexpr float kTrackHorizontalInset = 2.0f;
    static constexpr float kMinTrackWidth = 28.0f;
    static constexpr float kMaxTrackWidth = 36.0f;
    static constexpr float kTrackWidthRatio = 0.82f;
    static constexpr float kMinTitleWidth = 42.0f;
    static constexpr float kTitleWidthRatio = 0.9f;
    static constexpr float kMinTitleFontSize = 11.0f;
    static constexpr float kMaxTitleFontSize = 15.0f;
    static constexpr float kTitleFontSizeRatio = 0.47f;
    static constexpr float kOuterTrackInsetX = 3.0f;
    static constexpr float kOuterTrackInsetY = 4.0f;
    static constexpr float kOuterTrackRadiusRatio = 0.5f;
    static constexpr float kOuterTrackOutlineThickness = 1.8f;
    static constexpr float kInnerTrackInsetX = 3.8f;
    static constexpr float kInnerTrackInsetY = 5.2f;
    static constexpr float kInnerTrackRadiusRatio = 0.48f;
    static constexpr float kMinThumbHeight = 18.0f;
    static constexpr float kMaxThumbHeight = 28.0f;
    static constexpr float kThumbHeightDivisor = 2.5f;
    static constexpr float kMinLabelHeight = 18.0f;
    static constexpr float kMaxLabelHeight = 24.0f;
    static constexpr float kLabelHeightRatio = 0.2f;
    static constexpr float kMinLabelFontSize = 12.0f;
    static constexpr float kMaxLabelFontSize = 17.0f;
    static constexpr float kLabelFontSizeRatio = 0.18f;
    static constexpr float kMinThumbWidth = 14.0f;
    static constexpr float kThumbHorizontalInset = 2.0f;
    static constexpr float kThumbWidthRatio = 0.78f;
    static constexpr float kThumbRadiusRatio = 0.48f;
    static constexpr float kThumbOutlineThickness = 1.0f;
    static constexpr float kThumbNotchWidthRatio = 0.42f;
    static constexpr float kThumbNotchHeight = 2.8f;
    static constexpr float kThumbNotchRadius = 1.5f;

    Geometry calculateGeometry() const {
        Geometry geometry;
        geometry.bounds = getLocalBounds().toFloat().reduced(kBoundsInset);

        geometry.titleHeight = juce::jlimit(kMinTitleHeight, kMaxTitleHeight,
                                            geometry.bounds.getHeight() * kTitleHeightRatio);

        geometry.selectorArea = geometry.bounds;
        geometry.selectorArea.removeFromTop(geometry.titleHeight);
        geometry.selectorArea = geometry.selectorArea.reduced(0.0f, kSelectorVerticalInset);

        const float labelWidth = juce::jlimit(kMinLabelWidth, kMaxLabelWidth,
                                              geometry.selectorArea.getWidth() * kLabelWidthRatio);
        geometry.valueLabelArea = geometry.selectorArea.removeFromLeft(labelWidth);

        auto trackArea = geometry.selectorArea.reduced(kTrackHorizontalInset, 0.0f);
        const float trackWidth = juce::jlimit(kMinTrackWidth, kMaxTrackWidth,
                                              trackArea.getWidth() * kTrackWidthRatio);
        geometry.trackArea = trackArea.withSizeKeepingCentre(trackWidth, trackArea.getHeight());

        const float titleWidth = juce::jlimit(kMinTitleWidth, geometry.bounds.getWidth(),
                                              geometry.bounds.getWidth() * kTitleWidthRatio);
        const float titleX =
            juce::jlimit(geometry.bounds.getX(), geometry.bounds.getRight() - titleWidth,
                         geometry.trackArea.getCentreX() - (titleWidth * 0.5f));
        auto titleArea = juce::Rectangle<float>(titleX, geometry.bounds.getY(), titleWidth,
                                                geometry.titleHeight);
        geometry.titleTop = titleArea.removeFromTop(titleArea.getHeight() * 0.5f);
        geometry.titleBottom = titleArea;

        geometry.outerTrack = geometry.trackArea.reduced(kOuterTrackInsetX, kOuterTrackInsetY);
        geometry.innerTrack = geometry.outerTrack.reduced(kInnerTrackInsetX, kInnerTrackInsetY);

        geometry.thumbHeight = juce::jlimit(kMinThumbHeight, kMaxThumbHeight,
                                            geometry.innerTrack.getHeight() / kThumbHeightDivisor);
        geometry.labelHeight = juce::jlimit(kMinLabelHeight, kMaxLabelHeight,
                                            geometry.selectorArea.getHeight() * kLabelHeightRatio);
        return geometry;
    }

    float valueToY(const Geometry& geometry, float value) const {
        const float minValue = static_cast<float>(getMinimum());
        const float maxValue = static_cast<float>(getMaximum());
        const float valueRange = maxValue - minValue;
        const float normalised = valueRange > 0.0f
            ? juce::jlimit(0.0f, 1.0f, (value - minValue) / valueRange)
            : 0.0f;

        const float travelTop = geometry.innerTrack.getY() + (geometry.thumbHeight * 0.5f);
        const float travelBottom =
            geometry.innerTrack.getBottom() - (geometry.thumbHeight * 0.5f);
        return juce::jmap(normalised, 0.0f, 1.0f, travelBottom, travelTop);
    }

    static juce::String formatBoostLevelLabel(float valueDb) {
        const int roundedDb = juce::roundToInt(valueDb);
        return roundedDb > 0 ? "+" + juce::String(roundedDb) : juce::String(roundedDb);
    }

    void drawTitle(juce::Graphics& g, const Geometry& geometry) const {
        g.setColour(findColour(titleTextColourId));
        const float titleFontSize = juce::jlimit(kMinTitleFontSize, kMaxTitleFontSize,
                                                 geometry.titleHeight * kTitleFontSizeRatio);
        g.setFont(titleFontSize);
        g.setFont(g.getCurrentFont().boldened());
        g.drawText("BOOST", geometry.titleTop.toNearestInt(), juce::Justification::centred,
                   false);
        g.drawText("LEVEL", geometry.titleBottom.toNearestInt(), juce::Justification::centred,
                   false);
    }

    void drawTrack(juce::Graphics& g, const Geometry& geometry) const {
        g.setColour(findColour(trackOuterFillColourId));
        g.fillRoundedRectangle(geometry.outerTrack,
                               geometry.outerTrack.getWidth() * kOuterTrackRadiusRatio);

        g.setColour(findColour(trackOuterOutlineColourId));
        g.drawRoundedRectangle(geometry.outerTrack,
                               geometry.outerTrack.getWidth() * kOuterTrackRadiusRatio,
                               kOuterTrackOutlineThickness);

        g.setColour(findColour(trackInnerFillColourId));
        g.fillRoundedRectangle(geometry.innerTrack,
                               geometry.innerTrack.getWidth() * kInnerTrackRadiusRatio);
    }

    void drawLevelLabels(juce::Graphics& g, const Geometry& geometry) const {
        g.setColour(findColour(levelLabelTextColourId));
        const float labelFontSize = juce::jlimit(kMinLabelFontSize, kMaxLabelFontSize,
                                                 geometry.selectorArea.getHeight()
                                                     * kLabelFontSizeRatio);
        g.setFont(labelFontSize);
        g.setFont(g.getCurrentFont().boldened());

        const int labelX = juce::roundToInt(geometry.valueLabelArea.getX());
        const int labelWidth = juce::roundToInt(geometry.valueLabelArea.getWidth());
        const int labelHeight = juce::roundToInt(geometry.labelHeight);

        for (std::size_t index = 0; index < kNumBoostLevels; ++index) {
            const float levelValue = static_cast<float>(index);
            const float labelY = valueToY(geometry, levelValue)
                - (static_cast<float>(labelHeight) * 0.5f);
            g.drawFittedText(formatBoostLevelLabel(kBoostLevels[index]), labelX,
                             juce::roundToInt(labelY), labelWidth, labelHeight,
                             juce::Justification::centredRight, 1);
        }
    }

    void drawThumb(juce::Graphics& g, const Geometry& geometry) const {
        const float thumbWidth = juce::jlimit(kMinThumbWidth,
                                              geometry.innerTrack.getWidth() - kThumbHorizontalInset,
                                              geometry.innerTrack.getWidth() * kThumbWidthRatio);
        const float selectedY = valueToY(geometry, static_cast<float>(getValue()));
        auto thumbBounds = juce::Rectangle<float>(
            geometry.innerTrack.getCentreX() - (thumbWidth * 0.5f),
            selectedY - (geometry.thumbHeight * 0.5f), thumbWidth, geometry.thumbHeight);

        g.setGradientFill(juce::ColourGradient(
            findColour(thumbTopColourId), thumbBounds.getCentreX(), thumbBounds.getY(),
            findColour(thumbBottomColourId), thumbBounds.getCentreX(), thumbBounds.getBottom(),
            false));

        const float thumbRadius =
            juce::jmin(thumbBounds.getWidth(), thumbBounds.getHeight()) * kThumbRadiusRatio;
        g.fillRoundedRectangle(thumbBounds, thumbRadius);

        g.setColour(findColour(thumbOutlineColourId));
        g.drawRoundedRectangle(thumbBounds, thumbRadius, kThumbOutlineThickness);

        g.setColour(findColour(thumbNotchColourId));
        auto notch = thumbBounds.withSizeKeepingCentre(
            thumbBounds.getWidth() * kThumbNotchWidthRatio, kThumbNotchHeight);
        g.fillRoundedRectangle(notch, kThumbNotchRadius);
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
