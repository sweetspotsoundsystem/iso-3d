#include <Iso3D/PluginEditor.h>

#include <Iso3D/Constants.h>

namespace audio_plugin {

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef_(p) {
    setSize(720, 260);
    setLookAndFeel(&moogLookAndFeel_);

    auto setupKnob = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible(slider);
    };

    setupKnob(lowSlider_);
    setupKnob(midSlider_);
    setupKnob(highSlider_);
    addAndMakeVisible(boostSlider_);

    // APVTS attachments
    auto& apvts = processorRef_.getAPVTS();
    lowAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kLow, lowSlider_);
    midAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kMid, midSlider_);
    highAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kHigh, highSlider_);
    boostAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamID::kBoost, boostSlider_);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff808080));
}

void AudioPluginAudioProcessorEditor::resized() {
    auto content = getLocalBounds().reduced(12);
    auto boostColumn = content.removeFromRight(84);
    auto knobArea = content.reduced(0, 4);

    const int knobDiameter = juce::jmax(
        80, juce::jmin(knobArea.getHeight() - 8, (knobArea.getWidth() / 3) - 8));
    auto knobRow = knobArea.withHeight(knobDiameter);
    knobRow.setY(knobArea.getCentreY() - (knobDiameter / 2));

    const int knobSlotWidth = knobRow.getWidth() / 3;
    auto lowSlot = knobRow.removeFromLeft(knobSlotWidth);
    auto midSlot = knobRow.removeFromLeft(knobSlotWidth);
    auto highSlot = knobRow;

    lowSlider_.setBounds(lowSlot.withSizeKeepingCentre(knobDiameter, knobDiameter));
    midSlider_.setBounds(midSlot.withSizeKeepingCentre(knobDiameter, knobDiameter));
    highSlider_.setBounds(highSlot.withSizeKeepingCentre(knobDiameter, knobDiameter));

    const int boostWidth = juce::jlimit(54, 64, boostColumn.getWidth() - 8);
    const int boostHeight = juce::jlimit(106, 122, knobDiameter - 26);
    boostSlider_.setBounds(boostColumn.withSizeKeepingCentre(boostWidth, boostHeight));
}

}  // namespace audio_plugin
