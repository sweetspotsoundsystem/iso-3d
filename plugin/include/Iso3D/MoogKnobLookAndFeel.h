#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginAssets.h"

namespace audio_plugin {

class MoogKnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MoogKnobLookAndFeel() {
        // Load PNGs from binary data
        knobBg_ = juce::ImageCache::getFromMemory(assets::knob_bg_png,
                                                   static_cast<int>(assets::knob_bg_pngSize));
        knobCap_ = juce::ImageCache::getFromMemory(assets::knob_cap_png,
                                                    static_cast<int>(assets::knob_cap_pngSize));

        // Dark theme colours
        setColour(juce::Label::textColourId, juce::Colour(0xffe4e3e3));
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffcccccc));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff3a3436));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe4e3e3));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff555555));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffaaaaaa));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff2a2528));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe4e3e3));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff5a5255));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float /*rotaryStartAngle*/,
                          float /*rotaryEndAngle*/, juce::Slider& /*slider*/) override {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        auto side = juce::jmin(bounds.getWidth(), bounds.getHeight());
        auto knobBounds = bounds.withSizeKeepingCentre(side, side);

        // Draw static background (ticks + base)
        if (knobBg_.isValid()) {
            g.drawImage(knobBg_, knobBounds,
                        juce::RectanglePlacement::centred);
        }

        // Rotation: 270-degree sweep from -135 to +135 degrees
        constexpr float kStartAngle = -135.0f * (juce::MathConstants<float>::pi / 180.0f);
        constexpr float kEndAngle = 135.0f * (juce::MathConstants<float>::pi / 180.0f);
        float angle = kStartAngle + sliderPosProportional * (kEndAngle - kStartAngle);

        // Draw rotating cap
        if (knobCap_.isValid()) {
            auto cx = knobBounds.getCentreX();
            auto cy = knobBounds.getCentreY();

            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));
            g.drawImage(knobCap_, knobBounds,
                        juce::RectanglePlacement::centred);
            g.restoreState();
        }
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());
        g.drawText(label.getText(), label.getLocalBounds(),
                   label.getJustificationType(), true);
    }

private:
    juce::Image knobBg_;
    juce::Image knobCap_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoogKnobLookAndFeel)
};

}  // namespace audio_plugin
