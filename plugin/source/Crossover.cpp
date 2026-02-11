#include <Iso3D/Crossover.h>

namespace audio_plugin {

void Crossover::prepare(double sampleRate) {
    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 0;  // sample-by-sample processing
    spec.numChannels = static_cast<juce::uint32>(kNumChannels);

    lowMidSplit_.setCutoffFrequency(kLowMidCrossoverHz);
    lowMidSplit_.prepare(spec);
    lowMidSplit_.reset();

    midHighSplit_.setCutoffFrequency(kMidHighCrossoverHz);
    midHighSplit_.prepare(spec);
    midHighSplit_.reset();
}

BandSamples Crossover::processSample(int channel, float input) {
    float low = 0.0f;
    float hp1Out = 0.0f;
    lowMidSplit_.processSample(channel, input, low, hp1Out);

    float mid = 0.0f;
    float high = 0.0f;
    midHighSplit_.processSample(channel, hp1Out, mid, high);

    return {low, mid, high};
}

}  // namespace audio_plugin
