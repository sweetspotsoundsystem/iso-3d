#include <Iso3D/PluginProcessor.h>

#include <Iso3D/PluginEditor.h>

#include <cmath>

namespace audio_plugin {

namespace {

float dbToLinear(float dB) {
    if (dB <= kKillThresholdDb) return 0.0f;
    if (std::abs(dB) <= kUnityDeadZoneDb) return 1.0f;
    return std::pow(10.0f, dB / 20.0f);
}

}  // namespace

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(createBusesProperties()),
      apvts_(*this, nullptr, "Parameters", createParameterLayout()) {
    lowParam_ = apvts_.getRawParameterValue(ParamID::kLow);
    midParam_ = apvts_.getRawParameterValue(ParamID::kMid);
    highParam_ = apvts_.getRawParameterValue(ParamID::kHigh);
    boostParam_ = apvts_.getRawParameterValue(ParamID::kBoost);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Skew so that midpoint (0.5 / MIDI CC 64) = 0 dB, matching analog isolators
    auto bandRange = juce::NormalisableRange<float>(-100.0f, 12.0f, 0.1f);
    bandRange.setSkewForCentre(0.0f);


    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::kLow, 1}, "Low", bandRange, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::kMid, 1}, "Mid", bandRange, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::kHigh, 1}, "High", bandRange, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ParamID::kBoost, 1}, "Boost",
        juce::StringArray{"0 dB", "+6 dB", "+12 dB"}, 0));

    return layout;
}

juce::AudioProcessor::BusesProperties AudioPluginAudioProcessor::createBusesProperties() {
    return BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true);
}

const juce::String AudioPluginAudioProcessor::getName() const { return "Iso3D"; }
bool AudioPluginAudioProcessor::acceptsMidi() const { return false; }
bool AudioPluginAudioProcessor::producesMidi() const { return false; }
bool AudioPluginAudioProcessor::isMidiEffect() const { return false; }
double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginAudioProcessor::getNumPrograms() { return 1; }
int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }
void AudioPluginAudioProcessor::setCurrentProgram(int /*index*/) {}
const juce::String AudioPluginAudioProcessor::getProgramName(int /*index*/) { return {}; }
void AudioPluginAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    crossover_.prepare(sampleRate);

    smoothAlpha_ = 1.0f - std::exp(-1.0f / (kGainSmoothTimeSec * static_cast<float>(sampleRate)));
    smoothedLowGain_ = 1.0f;
    smoothedMidGain_ = 1.0f;
    smoothedHighGain_ = 1.0f;
}

void AudioPluginAudioProcessor::releaseResources() {}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& /*midiMessages*/) {
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters
    float boostMaxDb = kBoostLevels[static_cast<int>(boostParam_->load())];
    float lowDb = std::min(lowParam_->load(), boostMaxDb);
    float midDb = std::min(midParam_->load(), boostMaxDb);
    float highDb = std::min(highParam_->load(), boostMaxDb);

    float lowGainTarget = dbToLinear(lowDb);
    float midGainTarget = dbToLinear(midDb);
    float highGainTarget = dbToLinear(highDb);

    int numSamples = buffer.getNumSamples();
    int numChannels = std::min(static_cast<int>(totalNumInputChannels), kNumChannels);

    for (int s = 0; s < numSamples; ++s) {
        // Smooth gains (once per sample, shared across channels)
        smoothedLowGain_ += smoothAlpha_ * (lowGainTarget - smoothedLowGain_);
        smoothedMidGain_ += smoothAlpha_ * (midGainTarget - smoothedMidGain_);
        smoothedHighGain_ += smoothAlpha_ * (highGainTarget - smoothedHighGain_);

        for (int ch = 0; ch < numChannels; ++ch) {
            float input = buffer.getSample(ch, s);

            auto [low, mid, high] = crossover_.processSample(ch, input);

            low *= smoothedLowGain_;
            mid *= smoothedMidGain_;
            high *= smoothedHighGain_;

            buffer.setSample(ch, s, low + mid + high);
        }
    }
}

void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType()))
        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

bool AudioPluginAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
    return new AudioPluginAudioProcessorEditor(*this);
}

}  // namespace audio_plugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new audio_plugin::AudioPluginAudioProcessor();
}
