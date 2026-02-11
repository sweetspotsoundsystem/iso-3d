#include <Iso3D/PluginEditor.h>

#include <Iso3D/Constants.h>

namespace audio_plugin {

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef_(p) {
    setSize(440, 160);
    setLookAndFeel(&moogLookAndFeel_);

    auto setupKnob = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible(slider);
    };

    setupKnob(lowSlider_);
    setupKnob(midSlider_);
    setupKnob(highSlider_);

    // APVTS attachments
    auto& apvts = processorRef_.getAPVTS();
    lowAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kLow, lowSlider_);
    midAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kMid, midSlider_);
    highAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kHigh, highSlider_);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff808080));
}

void AudioPluginAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(10);
    int knobWidth = area.getWidth() / 3;

    lowSlider_.setBounds(area.removeFromLeft(knobWidth));
    midSlider_.setBounds(area.removeFromLeft(knobWidth));
    highSlider_.setBounds(area);
}

}  // namespace audio_plugin
