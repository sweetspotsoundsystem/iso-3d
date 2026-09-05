#pragma once

#include <array>
#include <functional>

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

// Real buttons keep visual bounds, hit targets, keyboard access and host gestures aligned.
class BoostSelector : public juce::Component {
public:
    explicit BoostSelector(juce::RangedAudioParameter& parameter)
        : attachment_(parameter, [this](float value) { updateSelection(value); }) {
        setComponentID("boost-selector");
        for (size_t i = 0; i < buttons_.size(); ++i) {
            auto& button = buttons_[i];
            const auto text = i == 0 ? juce::String("0 dB")
                                    : "+" + juce::String(kBoostLevels[i], 0) + " dB";
            button.setButtonText(text);
            button.setToggleable(true);
            button.setComponentID("boost-" + juce::String(static_cast<int>(i)));
            button.setTitle("Maximum boost " + text);
            button.setTooltip("Limit each band's boost to " + text + ". Cuts are unaffected.");
            button.onClick = [this, i] {
                attachment_.setValueAsCompleteGesture(static_cast<float>(i));
            };
            addAndMakeVisible(button);
        }
        attachment_.sendInitialUpdate();
    }

    void resized() override {
        const int buttonHeight = getHeight() / 3;
        for (size_t i = 0; i < buttons_.size(); ++i) {
            buttons_[i].setBounds(0, (2 - static_cast<int>(i)) * buttonHeight,
                                  getWidth(), buttonHeight);
        }
    }

    void paint(juce::Graphics& g) override {
        const float rowHeight = static_cast<float>(getHeight()) / 3.0f;
        const auto slot = juce::Rectangle<float>(6.0f, rowHeight * 0.5f - 9.0f,
                                                 10.0f, rowHeight * 2.0f + 18.0f);
        g.setColour(juce::Colour(0xff8a8d89));
        g.drawRoundedRectangle(slot.expanded(1.0f), 2.0f, 1.0f);
        g.setColour(juce::Colour(0xff323537));
        g.fillRoundedRectangle(slot, 1.0f);
    }

    float getMaximumBoostDb() const { return kBoostLevels[selectedIndex_]; }
    std::function<void()> onChange;

private:
    void updateSelection(float value) {
        selectedIndex_ = static_cast<size_t>(juce::jlimit(0, 2, juce::roundToInt(value)));
        for (size_t i = 0; i < buttons_.size(); ++i)
            buttons_[i].setToggleState(i == selectedIndex_, juce::dontSendNotification);
        if (onChange) onChange();
    }

    std::array<juce::TextButton, 3> buttons_;
    size_t selectedIndex_ = 0;
    juce::ParameterAttachment attachment_;
};

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void updateGainLimits();

    AudioPluginAudioProcessor& processorRef_;
    MoogKnobLookAndFeel moogLookAndFeel_;
    std::array<NotchedSlider, kNumBands> bandSliders_;
    std::array<juce::Label, kNumBands> bandLabels_;
    std::array<juce::Label, kNumBands> frequencyLabels_;
    std::array<juce::Label, kNumBands> limitLabels_;
    BoostSelector boostSelector_;
    juce::TooltipWindow tooltipWindow_{this, 650};
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               kNumBands> bandAttachments_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

}  // namespace audio_plugin
