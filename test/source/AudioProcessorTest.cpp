#include <gtest/gtest.h>

#include <Iso3D/Constants.h>
#include <Iso3D/Crossover.h>
#include <Iso3D/PluginProcessor.h>

#include <cmath>
#include <numbers>
#include <random>

using namespace audio_plugin;

namespace {

constexpr double kSampleRate = 48000.0;
constexpr int kWarmupSamples = 10000;
constexpr int kTestSamples = 10000;

float generateSine(float freq, int sampleIndex, double sampleRate) {
    return std::sin(2.0f * std::numbers::pi_v<float> * freq
                    * static_cast<float>(sampleIndex) / static_cast<float>(sampleRate));
}

float rmsLevel(const float* data, int numSamples) {
    double sum = 0.0;
    for (int i = 0; i < numSamples; ++i)
        sum += static_cast<double>(data[i]) * static_cast<double>(data[i]);
    return static_cast<float>(std::sqrt(sum / static_cast<double>(numSamples)));
}

float energyToDb(float ratio) {
    if (ratio < 1e-10f) return -100.0f;
    return 10.0f * std::log10(ratio);
}

// Helper to process a buffer in blocks of 512
void processInBlocks(AudioPluginAudioProcessor& processor, juce::AudioBuffer<float>& buffer,
                     int totalSamples) {
    juce::MidiBuffer midi;
    for (int pos = 0; pos < totalSamples; pos += 512) {
        int blockSize = std::min(512, totalSamples - pos);
        juce::AudioBuffer<float> block(buffer.getArrayOfWritePointers(), 2, pos, blockSize);
        processor.processBlock(block, midi);
    }
}

}  // namespace

// ===== Crossover Tests =====

TEST(CrossoverTest, BandsSumFlat) {
    Crossover xover;
    xover.prepare(kSampleRate);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // Warmup
    for (int i = 0; i < kWarmupSamples; ++i) {
        xover.processSample(0, dist(rng));
    }

    // Measure magnitude preservation (LP + HP = allpass, so magnitudes match
    // even though phase differs; we compare RMS levels, not sample values)
    std::vector<float> outputs(static_cast<size_t>(kTestSamples));
    std::vector<float> inputs(static_cast<size_t>(kTestSamples));
    for (size_t i = 0; i < static_cast<size_t>(kTestSamples); ++i) {
        float input = dist(rng);
        inputs[i] = input;
        auto [low, mid, high] = xover.processSample(0, input);
        outputs[i] = low + mid + high;
    }

    float outputRms = rmsLevel(outputs.data(), kTestSamples);
    float inputRms = rmsLevel(inputs.data(), kTestSamples);
    float magnitudeErrorDb = 20.0f * std::log10(outputRms / inputRms);

    EXPECT_NEAR(magnitudeErrorDb, 0.0f, 0.1f)
        << "Band sum should preserve magnitude, got " << magnitudeErrorDb << " dB difference";
}

TEST(CrossoverTest, LowFreqInLowBand) {
    Crossover xover;
    xover.prepare(kSampleRate);

    constexpr float kFreq = 50.0f;

    for (int i = 0; i < kWarmupSamples; ++i) {
        float s = generateSine(kFreq, i, kSampleRate);
        xover.processSample(0, s);
    }

    double lowEnergy = 0.0;
    double midEnergy = 0.0;
    double highEnergy = 0.0;
    for (int i = 0; i < kTestSamples; ++i) {
        float s = generateSine(kFreq, kWarmupSamples + i, kSampleRate);
        auto [low, mid, high] = xover.processSample(0, s);
        lowEnergy += static_cast<double>(low) * static_cast<double>(low);
        midEnergy += static_cast<double>(mid) * static_cast<double>(mid);
        highEnergy += static_cast<double>(high) * static_cast<double>(high);
    }

    double total = lowEnergy + midEnergy + highEnergy;
    EXPECT_GT(lowEnergy / total, 0.99)
        << "50Hz should be >99% in low band, got " << (lowEnergy / total * 100.0) << "%";
}

TEST(CrossoverTest, MidFreqInMidBand) {
    Crossover xover;
    xover.prepare(kSampleRate);

    constexpr float kFreq = 1000.0f;

    for (int i = 0; i < kWarmupSamples; ++i) {
        float s = generateSine(kFreq, i, kSampleRate);
        xover.processSample(0, s);
    }

    double lowEnergy = 0.0;
    double midEnergy = 0.0;
    double highEnergy = 0.0;
    for (int i = 0; i < kTestSamples; ++i) {
        float s = generateSine(kFreq, kWarmupSamples + i, kSampleRate);
        auto [low, mid, high] = xover.processSample(0, s);
        lowEnergy += static_cast<double>(low) * static_cast<double>(low);
        midEnergy += static_cast<double>(mid) * static_cast<double>(mid);
        highEnergy += static_cast<double>(high) * static_cast<double>(high);
    }

    double total = lowEnergy + midEnergy + highEnergy;
    EXPECT_GT(midEnergy / total, 0.95)
        << "1kHz should be >95% in mid band, got " << (midEnergy / total * 100.0) << "%";
}

TEST(CrossoverTest, HighFreqInHighBand) {
    Crossover xover;
    xover.prepare(kSampleRate);

    constexpr float kFreq = 10000.0f;

    for (int i = 0; i < kWarmupSamples; ++i) {
        float s = generateSine(kFreq, i, kSampleRate);
        xover.processSample(0, s);
    }

    double lowEnergy = 0.0;
    double midEnergy = 0.0;
    double highEnergy = 0.0;
    for (int i = 0; i < kTestSamples; ++i) {
        float s = generateSine(kFreq, kWarmupSamples + i, kSampleRate);
        auto [low, mid, high] = xover.processSample(0, s);
        lowEnergy += static_cast<double>(low) * static_cast<double>(low);
        midEnergy += static_cast<double>(mid) * static_cast<double>(mid);
        highEnergy += static_cast<double>(high) * static_cast<double>(high);
    }

    double total = lowEnergy + midEnergy + highEnergy;
    EXPECT_GT(highEnergy / total, 0.95)
        << "10kHz should be >95% in high band, got " << (highEnergy / total * 100.0) << "%";
}

TEST(CrossoverTest, CrossoverSlopeIs24dBPerOctave) {
    Crossover xover;
    xover.prepare(kSampleRate);

    // Test low/mid crossover (250Hz): measure low band energy at 500Hz (1 octave above)
    constexpr float kTestFreq = 500.0f;

    for (int i = 0; i < kWarmupSamples; ++i) {
        float s = generateSine(kTestFreq, i, kSampleRate);
        xover.processSample(0, s);
    }

    double lowEnergy = 0.0;
    double inputEnergy = 0.0;
    for (int i = 0; i < kTestSamples; ++i) {
        float s = generateSine(kTestFreq, kWarmupSamples + i, kSampleRate);
        auto [low, mid, high] = xover.processSample(0, s);
        (void)mid;
        (void)high;
        lowEnergy += static_cast<double>(low) * static_cast<double>(low);
        inputEnergy += static_cast<double>(s) * static_cast<double>(s);
    }

    float attenuationDb = energyToDb(static_cast<float>(lowEnergy / inputEnergy));
    EXPECT_NEAR(attenuationDb, -24.0f, 3.0f)
        << "Low band at 500Hz (1 oct above 250Hz crossover): " << attenuationDb << " dB";
}

// ===== Gain Tests =====

TEST(GainTest, KillBandRemovesSignal) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();
    processor->prepareToPlay(kSampleRate, 512);

    auto* lowParam = processor->getAPVTS().getParameter(ParamID::kLow);
    lowParam->setValueNotifyingHost(lowParam->convertTo0to1(-100.0f));

    constexpr float kFreq = 50.0f;
    constexpr int kTotalSamples = kWarmupSamples + kTestSamples;

    juce::AudioBuffer<float> buffer(2, kTotalSamples);
    for (int i = 0; i < kTotalSamples; ++i) {
        float s = generateSine(kFreq, i, kSampleRate);
        buffer.setSample(0, i, s);
        buffer.setSample(1, i, s);
    }

    processInBlocks(*processor, buffer, kTotalSamples);

    float outputRms = rmsLevel(buffer.getReadPointer(0) + kWarmupSamples, kTestSamples);
    // Generate reference input for the test region
    std::vector<float> refInput(static_cast<size_t>(kTestSamples));
    for (size_t i = 0; i < static_cast<size_t>(kTestSamples); ++i) {
        refInput[i] = generateSine(kFreq, kWarmupSamples + static_cast<int>(i), kSampleRate);
    }
    float inputRms = rmsLevel(refInput.data(), kTestSamples);
    float ratioDb = 20.0f * std::log10(outputRms / inputRms);

    // 50Hz is ~2.3 octaves below 250Hz crossover, so LR4 (24dB/oct) gives ~55dB attenuation
    EXPECT_LT(ratioDb, -40.0f)
        << "Killed low band should attenuate 50Hz signal, got " << ratioDb << " dB";
}

TEST(GainTest, UnityGainPassthrough) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();
    processor->prepareToPlay(kSampleRate, 512);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    constexpr int kTotalSamples = kWarmupSamples + kTestSamples;
    juce::AudioBuffer<float> buffer(2, kTotalSamples);
    std::vector<float> originalInput(static_cast<size_t>(kTotalSamples));

    for (size_t i = 0; i < static_cast<size_t>(kTotalSamples); ++i) {
        float s = dist(rng);
        originalInput[i] = s;
        buffer.setSample(0, static_cast<int>(i), s);
        buffer.setSample(1, static_cast<int>(i), s);
    }

    processInBlocks(*processor, buffer, kTotalSamples);

    // Compare RMS levels (crossover introduces allpass phase shift, so sample
    // values differ but magnitude should be preserved)
    float outputRms = rmsLevel(buffer.getReadPointer(0) + kWarmupSamples, kTestSamples);
    float inputRms = rmsLevel(originalInput.data() + kWarmupSamples, kTestSamples);
    float magnitudeErrorDb = 20.0f * std::log10(outputRms / inputRms);

    EXPECT_NEAR(magnitudeErrorDb, 0.0f, 0.1f)
        << "Unity gain passthrough should preserve magnitude, got " << magnitudeErrorDb << " dB";
}

TEST(GainTest, SilenceInSilenceOut) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();
    processor->prepareToPlay(kSampleRate, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();

    juce::MidiBuffer midi;
    processor->processBlock(buffer, midi);

    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 512; ++i) {
            EXPECT_LT(std::abs(buffer.getSample(ch, i)), 1e-7f)
                << "Non-silence at ch=" << ch << " sample=" << i;
        }
    }
}

TEST(GainTest, GainSmoothingNoClicks) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();
    processor->prepareToPlay(kSampleRate, 512);

    constexpr float kFreq = 1000.0f;

    // Process warmup at unity gain
    juce::AudioBuffer<float> warmup(2, kWarmupSamples);
    for (int i = 0; i < kWarmupSamples; ++i) {
        float s = generateSine(kFreq, i, kSampleRate);
        warmup.setSample(0, i, s);
        warmup.setSample(1, i, s);
    }
    processInBlocks(*processor, warmup, kWarmupSamples);

    // Kill mid band and process more audio
    auto* midParam = processor->getAPVTS().getParameter(ParamID::kMid);
    midParam->setValueNotifyingHost(midParam->convertTo0to1(-100.0f));

    constexpr int kPostChangeSamples = 4096;
    juce::AudioBuffer<float> postChange(2, kPostChangeSamples);
    for (int i = 0; i < kPostChangeSamples; ++i) {
        float s = generateSine(kFreq, kWarmupSamples + i, kSampleRate);
        postChange.setSample(0, i, s);
        postChange.setSample(1, i, s);
    }
    processInBlocks(*processor, postChange, kPostChangeSamples);

    // Verify no sample exceeds the pre-change level
    for (int i = 0; i < kPostChangeSamples; ++i) {
        EXPECT_LE(std::abs(postChange.getSample(0, i)), 1.01f)
            << "Click/overshoot at sample " << i << ": " << postChange.getSample(0, i);
    }
}

TEST(GainTest, BoostClamping) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();
    processor->prepareToPlay(kSampleRate, 512);

    // Set band to +12dB, boost to 0dB -> effective should be 0dB
    auto& apvts = processor->getAPVTS();
    auto* lowParam = apvts.getParameter(ParamID::kLow);
    lowParam->setValueNotifyingHost(lowParam->convertTo0to1(12.0f));
    // boost stays at index 0 = "0 dB"

    constexpr float kFreq = 50.0f;
    constexpr int kTotalSamples = kWarmupSamples + kTestSamples;

    juce::AudioBuffer<float> buffer(2, kTotalSamples);
    std::vector<float> originalInput(static_cast<size_t>(kTotalSamples));
    for (size_t i = 0; i < static_cast<size_t>(kTotalSamples); ++i) {
        float s = generateSine(kFreq, static_cast<int>(i), kSampleRate);
        originalInput[i] = s;
        buffer.setSample(0, static_cast<int>(i), s);
        buffer.setSample(1, static_cast<int>(i), s);
    }

    processInBlocks(*processor, buffer, kTotalSamples);

    // Measure output vs input energy ratio (should be near unity = 0 dB)
    float outputRms = rmsLevel(buffer.getReadPointer(0) + kWarmupSamples, kTestSamples);
    float inputRms = rmsLevel(originalInput.data() + kWarmupSamples, kTestSamples);
    float ratioDb = 20.0f * std::log10(outputRms / inputRms);

    EXPECT_NEAR(ratioDb, 0.0f, 0.5f)
        << "With +12dB gain but 0dB boost limit, effective should be ~0dB, got " << ratioDb;
}

// ===== Plugin Tests =====

TEST(PluginTest, PluginInstantiation) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();
    EXPECT_EQ(processor->getName(), "Iso3D");
}

TEST(PluginTest, BusLayout) {
    auto processor = std::make_unique<AudioPluginAudioProcessor>();

    // Stereo in/out should be accepted
    juce::AudioProcessor::BusesLayout stereoLayout;
    stereoLayout.inputBuses.add(juce::AudioChannelSet::stereo());
    stereoLayout.outputBuses.add(juce::AudioChannelSet::stereo());
    EXPECT_TRUE(processor->isBusesLayoutSupported(stereoLayout));

    // Mono should be rejected
    juce::AudioProcessor::BusesLayout monoLayout;
    monoLayout.inputBuses.add(juce::AudioChannelSet::mono());
    monoLayout.outputBuses.add(juce::AudioChannelSet::mono());
    EXPECT_FALSE(processor->isBusesLayoutSupported(monoLayout));

    // Mono in, stereo out should be rejected
    juce::AudioProcessor::BusesLayout mixedLayout;
    mixedLayout.inputBuses.add(juce::AudioChannelSet::mono());
    mixedLayout.outputBuses.add(juce::AudioChannelSet::stereo());
    EXPECT_FALSE(processor->isBusesLayoutSupported(mixedLayout));
}
