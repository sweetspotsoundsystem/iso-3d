#include <gtest/gtest.h>
#include <Iso3D/PluginEditor.h>

using namespace audio_plugin;

namespace {

class GestureListener : public juce::AudioProcessorParameter::Listener {
public:
    void parameterValueChanged(int, float) override {}
    void parameterGestureChanged(int, bool starting) override {
        if (starting) ++starts;
        else ++ends;
    }
    int starts = 0;
    int ends = 0;
};

void clickButton(juce::Button& button) {
    const auto position = button.getLocalBounds().toFloat().getCentre();
    const auto now = juce::Time::getCurrentTime();
    const juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(), position,
                                 juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),
                                 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                 &button, &button, now, position, now, 1, false);
    juce::Component& component = button;
    component.mouseDown(event);
    component.mouseUp(event.withNewPosition(position));
}

}  // namespace

TEST(EditorTest, BoostButtonsSelectTheirLabelAndNotifyHost) {
    AudioPluginAudioProcessor processor;
    auto* parameter = processor.getAPVTS().getParameter(ParamID::kBoost);
    GestureListener listener;
    parameter->addListener(&listener);
    {
        AudioPluginAudioProcessorEditor editor(processor);
        editor.setVisible(true);
        auto* selector = editor.findChildWithID("boost-selector");
        ASSERT_NE(selector, nullptr);
        for (const int index : {2, 1, 0}) {
            auto* button = dynamic_cast<juce::Button*>(selector->findChildWithID("boost-" + juce::String(index)));
            ASSERT_NE(button, nullptr);
            const auto centre = editor.getLocalPoint(button, button->getLocalBounds().getCentre());
            EXPECT_EQ(editor.getComponentAt(centre), button);
            clickButton(*button);
            EXPECT_FLOAT_EQ(processor.getAPVTS().getRawParameterValue(ParamID::kBoost)->load(),
                            static_cast<float>(index));
            EXPECT_TRUE(button->getToggleState());
        }
        EXPECT_EQ(listener.starts, 3);
        EXPECT_EQ(listener.ends, 3);
    }
    parameter->removeListener(&listener);
}

TEST(EditorTest, HostAutomationUpdatesBoostButtonsAndGainReadouts) {
    AudioPluginAudioProcessor processor;
    AudioPluginAudioProcessorEditor editor(processor);
    auto& apvts = processor.getAPVTS();
    auto* parameter = apvts.getParameter(ParamID::kBoost);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(2.0f));
    auto* selector = editor.findChildWithID("boost-selector");
    ASSERT_NE(selector, nullptr);
    for (int index = 0; index < 3; ++index) {
        auto* button = dynamic_cast<juce::Button*>(selector->findChildWithID("boost-" + juce::String(index)));
        ASSERT_NE(button, nullptr);
        EXPECT_EQ(button->getToggleState(), index == 2);
    }
    auto* low = apvts.getParameter(ParamID::kLow);
    low->setValueNotifyingHost(low->convertTo0to1(-100.0f));
    auto* slider = dynamic_cast<juce::Slider*>(editor.findChildWithID("band-low"));
    ASSERT_NE(slider, nullptr);
    EXPECT_DOUBLE_EQ(slider->getValue(), -100.0);
    EXPECT_EQ(slider->getTextFromValue(slider->getValue()), "KILL");
    EXPECT_DOUBLE_EQ(slider->getValueFromText("KILL"), -100.0);
    EXPECT_DOUBLE_EQ(slider->getValueFromText("-6.0 dB"), -6.0);
}
