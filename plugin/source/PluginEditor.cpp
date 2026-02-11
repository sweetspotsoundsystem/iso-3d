#include <Iso3D/PluginEditor.h>

#include <Iso3D/Constants.h>

namespace audio_plugin {

namespace {

constexpr int kEditorWidth = 720;
constexpr int kEditorHeight = 260;
constexpr int kEditorMargin = 12;
constexpr int kBoostColumnWidth = 84;
constexpr int kKnobAreaVerticalInset = 4;
constexpr int kMinKnobDiameter = 80;
constexpr int kKnobSlotPadding = 8;
constexpr int kBoostWidthMin = 54;
constexpr int kBoostWidthMax = 64;
constexpr int kBoostWidthInset = 8;
constexpr int kBoostHeightMin = 106;
constexpr int kBoostHeightMax = 122;
constexpr int kBoostHeightFromKnobOffset = 26;

}  // namespace

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef_(p) {
    setSize(kEditorWidth, kEditorHeight);
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
    auto content = getLocalBounds().reduced(kEditorMargin);
    auto boostColumn = content.removeFromRight(kBoostColumnWidth);
    auto knobArea = content.reduced(0, kKnobAreaVerticalInset);

    const int knobDiameter = juce::jmax(kMinKnobDiameter,
                                        juce::jmin(knobArea.getHeight() - kKnobSlotPadding,
                                                   (knobArea.getWidth() / 3) - kKnobSlotPadding));
    auto knobRow = knobArea.withHeight(knobDiameter);
    knobRow.setY(knobArea.getCentreY() - (knobDiameter / 2));

    const int knobSlotWidth = knobRow.getWidth() / 3;
    auto lowSlot = knobRow.removeFromLeft(knobSlotWidth);
    auto midSlot = knobRow.removeFromLeft(knobSlotWidth);
    auto highSlot = knobRow;

    lowSlider_.setBounds(lowSlot.withSizeKeepingCentre(knobDiameter, knobDiameter));
    midSlider_.setBounds(midSlot.withSizeKeepingCentre(knobDiameter, knobDiameter));
    highSlider_.setBounds(highSlot.withSizeKeepingCentre(knobDiameter, knobDiameter));

    const int boostWidth =
        juce::jlimit(kBoostWidthMin, kBoostWidthMax, boostColumn.getWidth() - kBoostWidthInset);
    const int boostHeight = juce::jlimit(kBoostHeightMin, kBoostHeightMax,
                                         knobDiameter - kBoostHeightFromKnobOffset);
    boostSlider_.setBounds(boostColumn.withSizeKeepingCentre(boostWidth, boostHeight));
}

}  // namespace audio_plugin
