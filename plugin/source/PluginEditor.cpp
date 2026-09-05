#include <Iso3D/PluginEditor.h>

namespace audio_plugin {

namespace {

constexpr int kEditorWidth = 860;
constexpr int kEditorHeight = 254;
constexpr int kBandStart = 186;
constexpr int kBandWidth = 178;
constexpr int kBoostX = 758;

const std::array<const char*, kNumBands> kBandNames{"LOW", "MID", "HIGH"};
const std::array<const char*, kNumBands> kFrequencyNames{
    "BELOW 250 Hz", "250 Hz - 3.14 kHz", "ABOVE 3.14 kHz"};
const std::array<const char*, kNumBands> kBandIDs{ParamID::kLow, ParamID::kMid, ParamID::kHigh};

juce::String gainText(double value) {
    if (value <= static_cast<double>(kKillThresholdDb)) return "KILL";
    if (std::abs(value) <= static_cast<double>(kUnityDeadZoneDb)) return "0.0 dB";
    return (value > 0.0 ? "+" : "") + juce::String(value, 1) + " dB";
}

void setupLabel(juce::Label& label, const juce::String& text, float size,
                juce::Colour colour) {
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(size)));
    label.setColour(juce::Label::textColourId, colour);
    label.setJustificationType(juce::Justification::centred);
    label.setInterceptsMouseClicks(false, false);
}

}  // namespace

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef_(p),
      boostSelector_(*p.getAPVTS().getParameter(ParamID::kBoost)) {
    setLookAndFeel(&moogLookAndFeel_);

    for (size_t i = 0; i < bandSliders_.size(); ++i) {
        auto& slider = bandSliders_[i];
        slider.setComponentID("band-" + juce::String(kBandIDs[i]));
        slider.setName(juce::String(kBandNames[i]) + " gain");
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 88, 22);
        slider.setMouseDragSensitivity(240);
        slider.setScrollWheelEnabled(false);
        slider.setTooltip("Drag to isolate. Double-click for 0 dB. Click the value to type a gain, or KILL.");
        slider.onValueChange = [this] { updateGainLimits(); };
        addAndMakeVisible(slider);

        setupLabel(bandLabels_[i], kBandNames[i], 12.0f, editor_colours::text);
        bandLabels_[i].setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        setupLabel(frequencyLabels_[i], kFrequencyNames[i], 9.0f, editor_colours::muted);
        setupLabel(limitLabels_[i], {}, 9.0f, editor_colours::accent);
        addAndMakeVisible(bandLabels_[i]);
        addAndMakeVisible(frequencyLabels_[i]);
        addAndMakeVisible(limitLabels_[i]);

        bandAttachments_[i] =
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                processorRef_.getAPVTS(), kBandIDs[i], slider);
        // Attachments install their own text conversion; apply display formatting afterwards.
        slider.textFromValueFunction = gainText;
        slider.valueFromTextFunction = [](const juce::String& text) {
            if (text.trim().equalsIgnoreCase("KILL"))
                return static_cast<double>(kKillThresholdDb);
            return text.getDoubleValue();
        };
        slider.updateText();
    }

    addAndMakeVisible(boostSelector_);
    boostSelector_.onChange = [this] { updateGainLimits(); };
    updateGainLimits();
    setSize(kEditorWidth, kEditorHeight);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::updateGainLimits() {
    const auto maximum = static_cast<double>(boostSelector_.getMaximumBoostDb());
    for (size_t i = 0; i < bandSliders_.size(); ++i) {
        const bool limited = bandSliders_[i].getValue() > maximum;
        limitLabels_[i].setText(limited ? "MAX " + gainText(maximum) : juce::String(),
                                juce::dontSendNotification);
        frequencyLabels_[i].setVisible(!limited);
    }
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff222426));
    const auto plate = getLocalBounds().toFloat().reduced(10.0f, 8.0f);
    juce::ColourGradient metal(juce::Colour(0xffdfe1e2), 0.0f, plate.getY(),
                               juce::Colour(0xffb7babd), 0.0f, plate.getBottom(), false);
    metal.addColour(0.24, editor_colours::panel);
    metal.addColour(0.67, juce::Colour(0xffd0d2d3));
    g.setGradientFill(metal);
    g.fillRect(plate);

    // A fine horizontal finish, drawn at component scale so it remains crisp at HiDPI.
    for (int row = 10; row < getHeight() - 8; row += 2) {
        const float alpha = row % 6 == 0 ? 0.022f : 0.01f;
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.drawHorizontalLine(row, plate.getX(), plate.getRight());
    }
    g.setColour(juce::Colour(0xffeeefec));
    g.drawHorizontalLine(8, 10.0f, static_cast<float>(getWidth()) - 10.0f);
    g.setColour(juce::Colour(0xff92958f));
    g.drawHorizontalLine(getHeight() - 9, 10.0f, static_cast<float>(getWidth()) - 10.0f);

    for (const float x : {23.0f, static_cast<float>(getWidth()) - 23.0f}) {
        for (const float y : {22.0f, static_cast<float>(getHeight()) - 22.0f}) {
            g.setColour(juce::Colour(0xff969992));
            g.fillEllipse(x - 3.6f, y - 3.6f, 7.2f, 7.2f);
            g.setColour(juce::Colour(0xffdaddd5));
            g.drawEllipse(x - 3.0f, y - 3.0f, 6.0f, 6.0f, 0.6f);
            g.setColour(juce::Colour(0xff5d625b));
            g.drawLine(x - 1.8f, y + 1.0f, x + 1.8f, y - 1.0f, 1.0f);
        }
    }

    g.setColour(editor_colours::text);
    auto modelFont = juce::Font(juce::FontOptions(26.0f).withStyle("Bold"));
    modelFont.setHorizontalScale(0.92f);
    g.setFont(modelFont);
    g.drawText("ISO 3D", 44, 105, 134, 33, juce::Justification::centredLeft);

    g.setColour(editor_colours::text);
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("BOOST", kBoostX - 1, 73, 78, 16, juce::Justification::centredLeft);
}

void AudioPluginAudioProcessorEditor::resized() {
    for (size_t i = 0; i < bandSliders_.size(); ++i) {
        const int x = kBandStart + static_cast<int>(i) * kBandWidth;
        bandLabels_[i].setBounds(x, 29, kBandWidth, 17);
        bandSliders_[i].setBounds(x + 3, 49, kBandWidth - 6, 167);
        frequencyLabels_[i].setBounds(x, 219, kBandWidth, 15);
        limitLabels_[i].setBounds(x, 219, kBandWidth, 15);
    }
    boostSelector_.setBounds(kBoostX - 3, 96, 78, 81);
}

}  // namespace audio_plugin
