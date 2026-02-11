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

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

}  // namespace audio_plugin
