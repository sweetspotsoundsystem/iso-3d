#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Constants.h"
#include "Crossover.h"

namespace audio_plugin {

class AudioPluginAudioProcessor : public juce::AudioProcessor {
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static BusesProperties createBusesProperties();

    juce::AudioProcessorValueTreeState apvts_;

    Crossover crossover_;

    // Parameter pointers for lock-free access in audio thread
    std::atomic<float>* lowParam_ = nullptr;
    std::atomic<float>* midParam_ = nullptr;
    std::atomic<float>* highParam_ = nullptr;
    std::atomic<float>* boostParam_ = nullptr;

    // Smoothed gain values (linear)
    float smoothedLowGain_ = 1.0f;
    float smoothedMidGain_ = 1.0f;
    float smoothedHighGain_ = 1.0f;

    // Smoothing coefficient, computed in prepareToPlay
    float smoothAlpha_ = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};

}  // namespace audio_plugin
