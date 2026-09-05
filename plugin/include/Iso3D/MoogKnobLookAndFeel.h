#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

namespace editor_colours {
inline const juce::Colour panel{0xffc8cacc};
inline const juce::Colour text{0xff26282a};
inline const juce::Colour muted{0xff5e6265};
inline const juce::Colour accent{0xff913c32};
inline const juce::Colour border{0xff989b96};
}  // namespace editor_colours

class MoogKnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MoogKnobLookAndFeel() {
        setColour(juce::Label::textColourId, editor_colours::text);
        setColour(juce::Slider::textBoxTextColourId, editor_colours::text);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TextEditor::textColourId, editor_colours::text);
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xffdedfdb));
        setColour(juce::TextEditor::highlightColourId, juce::Colour(0xffafb8c0));
        setColour(juce::TextEditor::focusedOutlineColourId, editor_colours::text);
        setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xffe8e9e5));
        setColour(juce::TooltipWindow::textColourId, editor_colours::text);
        setColour(juce::TooltipWindow::outlineColourId, editor_colours::border);
    }

    juce::Label* createSliderTextBox(juce::Slider& slider) override {
        auto* label = juce::LookAndFeel_V4::createSliderTextBox(slider);
        label->setFont(juce::Font(juce::FontOptions(12.0f)));
        label->setJustificationType(juce::Justification::centred);
        return label;
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override {
        if (!label.isEditable()) {
            juce::LookAndFeel_V4::drawLabel(g, label);
            return;
        }
        if (label.isMouseOver() || label.hasKeyboardFocus(true)) {
            g.setColour(editor_colours::muted);
            g.drawHorizontalLine(label.getHeight() - 2, 8.0f,
                                  static_cast<float>(label.getWidth()) - 8.0f);
        }
        if (!label.isBeingEdited()) {
            g.setColour(editor_colours::text);
            g.setFont(label.getFont());
            g.drawText(label.getText(), label.getLocalBounds(), juce::Justification::centred);
        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                               const juce::Colour&, bool highlighted, bool down) override {
        const float cy = static_cast<float>(button.getHeight()) * 0.5f;
        if (button.getToggleState()) {
            const juce::Rectangle<float> thumb(2.0f, cy - 8.0f, 18.0f, 16.0f);
            g.setColour(juce::Colour(0xff151719));
            g.fillRoundedRectangle(thumb.translated(0.0f, 1.0f), 2.0f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff626568), 0.0f, cy - 8.0f,
                                                  juce::Colour(0xff25272a), 0.0f, cy + 8.0f, false));
            g.fillRoundedRectangle(thumb, 2.0f);
            g.setColour(juce::Colour(0xff808386));
            g.drawRoundedRectangle(thumb.reduced(0.5f), 2.0f, 0.75f);
            g.setColour(juce::Colour(0xff16181a));
            for (int i = -1; i <= 1; ++i) {
                const float y = cy + static_cast<float>(i) * 3.0f;
                g.drawLine(5.0f, y, 17.0f, y, 1.0f);
            }
        }
        if (highlighted || down || button.hasKeyboardFocus(true)) {
            g.setColour(editor_colours::text.withAlpha(0.28f));
            g.drawRect(button.getLocalBounds().toFloat().reduced(0.5f), 0.75f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override {
        g.setColour(editor_colours::text);
        g.setFont(juce::FontOptions(11.0f));
        g.drawText(button.getButtonText(), button.getLocalBounds().withTrimmedLeft(29),
                   juce::Justification::centredLeft);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float position, float, float, juce::Slider& slider) override {
        const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        const float side = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const float cx = bounds.getCentreX();
        const float cy = bounds.getCentreY();
        const float radius = side * 0.325f;
        const float scaleRadius = side * 0.415f;
        constexpr float pi = juce::MathConstants<float>::pi;
        constexpr float start = -0.75f * pi;
        constexpr float sweep = 1.5f * pi;
        const float angle = start + position * sweep;
        const auto point = [cx, cy](float a, float r) {
            return juce::Point<float>(cx + std::sin(a) * r, cy - std::cos(a) * r);
        };

        // Fixed panel graduations follow physical knob travel; zero is the centre detent.
        for (int i = 0; i <= 12; ++i) {
            const float a = start + static_cast<float>(i) / 12.0f * sweep;
            const bool major = i == 0 || i == 6 || i == 12;
            g.setColour(major ? editor_colours::text : editor_colours::muted);
            g.drawLine(juce::Line<float>(point(a, scaleRadius - (major ? 6.0f : 3.0f)),
                                         point(a, scaleRadius)), major ? 1.4f : 0.8f);
        }
        g.setFont(juce::FontOptions(9.0f));
        g.setColour(editor_colours::text);
        g.drawText("0", juce::Rectangle<float>(cx - 12.0f, cy - scaleRadius - 13.0f, 24.0f, 12.0f),
                   juce::Justification::centred);
        const auto cutPosition = point(start, scaleRadius + 7.0f);
        const auto boostPosition = point(start + sweep, scaleRadius + 7.0f);
        g.drawText(juce::String::fromUTF8("\xe2\x88\x92\xe2\x88\x9e"),
                   juce::Rectangle<float>(cutPosition.x - 16.0f, cutPosition.y + 1.0f, 28.0f, 12.0f),
                   juce::Justification::centred);
        g.drawText("+12", juce::Rectangle<float>(boostPosition.x - 12.0f, boostPosition.y + 1.0f, 28.0f, 12.0f),
                   juce::Justification::centred);

        const juce::Rectangle<float> body(cx - radius, cy - radius, 2.0f * radius, 2.0f * radius);
        for (int i = 3; i >= 1; --i) {
            g.setColour(juce::Colours::black.withAlpha(0.045f));
            g.fillEllipse(body.expanded(static_cast<float>(i)).translated(0.0f, 2.0f));
        }

        // Rounded finger grips turn with the pointer; the lighting stays fixed to the panel.
        const auto gripOutline = [&](float r) {
            juce::Path outline;
            constexpr int samples = 240;
            for (int i = 0; i <= samples; ++i) {
                const float a = static_cast<float>(i) * (2.0f * pi / static_cast<float>(samples));
                const float gripRadius = r * (0.98f - 0.02f * std::cos(10.0f * (a - angle)));
                const auto p = point(a, gripRadius);
                if (i == 0) outline.startNewSubPath(p);
                else outline.lineTo(p);
            }
            outline.closeSubPath();
            return outline;
        };
        const auto grip = gripOutline(radius);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff53585f), cx - radius * 0.4f, cy - radius,
                                              juce::Colour(0xff101215), cx + radius * 0.3f, cy + radius, false));
        g.fillPath(grip);
        g.setColour(juce::Colour(0xff17191c));
        g.strokePath(grip, juce::PathStrokeType(0.7f));

        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff35393f), cx, cy - radius,
                                              juce::Colour(0xff1c1e22), cx, cy + radius, false));
        g.fillPath(gripOutline(radius - 2.5f));

        // Broad, recessed grooves give the black rim its moulded grip profile.
        for (int i = 0; i < 10; ++i) {
            const float a = angle + static_cast<float>(i) * (2.0f * pi / 10.0f);
            juce::Path groove;
            groove.startNewSubPath(point(a - 0.10f, radius * 0.935f));
            groove.quadraticTo(point(a, radius * 0.78f), point(a + 0.10f, radius * 0.935f));
            g.setColour(juce::Colours::black.withAlpha(0.30f));
            g.strokePath(groove, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
        }

        const float capRadius = radius * 0.77f;
        const juce::Rectangle<float> cap(cx - capRadius, cy - capRadius,
                                         2.0f * capRadius, 2.0f * capRadius);
        g.setColour(juce::Colour(0xff0d0f12));
        g.fillEllipse(cap.expanded(0.8f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff282b30), cap.getX(), cap.getY(),
                                              juce::Colour(0xff1b1d21), cap.getRight(), cap.getBottom(), false));
        g.fillEllipse(cap);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff51565d), cx, cap.getY(),
                                              juce::Colour(0xff292c31), cx, cap.getBottom(), false));
        g.drawEllipse(cap.reduced(0.35f), 0.6f);

        juce::Path pointer;
        pointer.startNewSubPath(point(angle, radius * 0.08f));
        pointer.lineTo(point(angle, radius * 0.93f));
        g.setColour(juce::Colour(0xffeeefed));
        g.strokePath(pointer, juce::PathStrokeType(2.7f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        if (slider.hasKeyboardFocus(true)) {
            g.setColour(editor_colours::text.withAlpha(0.5f));
            g.drawEllipse(body.expanded(5.0f), 1.0f);
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoogKnobLookAndFeel)
};

}  // namespace audio_plugin
