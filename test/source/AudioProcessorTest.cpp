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
    AudioPluginAudioProcessor processor;
    processor.prepareToPlay(kSampleRate, 512);

    // A settled DC signal isolates gain discontinuities from waveform changes.
    juce::AudioBuffer<float> warmup(2, kWarmupSamples);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < warmup.getNumSamples(); ++i)
            warmup.setSample(ch, i, 1.0f);
    processInBlocks(processor, warmup, kWarmupSamples);
    float previous = warmup.getSample(0, kWarmupSamples - 1);
    EXPECT_NEAR(previous, 1.0f, 0.001f);

    for (const auto* id : {ParamID::kLow, ParamID::kMid, ParamID::kHigh}) {
        auto* parameter = processor.getAPVTS().getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(-100.0f));
    }
    juce::AudioBuffer<float> fade(2, 4096);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < fade.getNumSamples(); ++i)
            fade.setSample(ch, i, 1.0f);
    processInBlocks(processor, fade, fade.getNumSamples());

    EXPECT_GT(fade.getSample(0, 0), 0.9f);  // The runtime change must fade, not jump.
    for (int i = 0; i < fade.getNumSamples(); ++i) {
        const float current = fade.getSample(0, i);
        EXPECT_LT(std::abs(current - previous), 0.01f);
        previous = current;
    }
    EXPECT_LT(std::abs(previous), 1e-5f);  // It must actually reach silence.
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

TEST(CrossoverTest, UnityResponseAcrossFrequenciesAndSampleRates) {
    for (const double sampleRate : {44100.0, 48000.0, 96000.0}) {
        for (const double frequency : {50.0, 250.0, 329.0, 500.0, 1000.0, 3140.0, 10000.0}) {
            Crossover crossover;
            crossover.prepare(sampleRate);
            const int measureSamples = static_cast<int>(sampleRate);
            double inputEnergy = 0.0;
            double outputEnergy = 0.0;
            for (int i = 0; i < kWarmupSamples + measureSamples; ++i) {
                const float input = static_cast<float>(std::sin(
                    2.0 * std::numbers::pi * frequency * static_cast<double>(i) / sampleRate));
                const auto [low, mid, high] = crossover.processSample(0, input);
                if (i >= kWarmupSamples) {
                    const double output = static_cast<double>(low + mid + high);
                    inputEnergy += static_cast<double>(input) * static_cast<double>(input);
                    outputEnergy += output * output;
                }
            }
            EXPECT_NEAR(10.0 * std::log10(outputEnergy / inputEnergy), 0.0, 0.005)
                << "Frequency: " << frequency << ", sample rate: " << sampleRate;
        }
    }
}

TEST(GainTest, RestoredKillIsSilentFromFirstSample) {
    AudioPluginAudioProcessor saved;
    for (const auto* id : {ParamID::kLow, ParamID::kMid, ParamID::kHigh}) {
        auto* parameter = saved.getAPVTS().getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(-100.0f));
    }
    juce::MemoryBlock state;
    saved.getStateInformation(state);

    // Both host lifecycle orders must respect the saved mute, including a zero-size block.
    for (const bool restoreBeforePrepare : {false, true}) {
        AudioPluginAudioProcessor processor;
        if (restoreBeforePrepare)
            processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        processor.prepareToPlay(kSampleRate, 512);
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> empty(2, 0);
        processor.processBlock(empty, midi);
        if (!restoreBeforePrepare)
            processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));

        for (int restart = 0; restart < 2; ++restart) {
            if (restart != 0) processor.prepareToPlay(96000.0, 512);
            juce::AudioBuffer<float> buffer(2, 512);
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                    buffer.setSample(ch, i, generateSine(1000.0f, i + 12, kSampleRate));
            processor.processBlock(buffer, midi);
            EXPECT_FLOAT_EQ(buffer.getMagnitude(0, buffer.getNumSamples()), 0.0f);
        }
    }
}

TEST(GainTest, InitialGainRespectsCutsAndBoostLimit) {
    for (const float requestedDb : {-12.0f, 12.0f}) {
        AudioPluginAudioProcessor processor;
        for (const auto* id : {ParamID::kLow, ParamID::kMid, ParamID::kHigh}) {
            auto* parameter = processor.getAPVTS().getParameter(id);
            parameter->setValueNotifyingHost(parameter->convertTo0to1(requestedDb));
        }
        auto* boost = processor.getAPVTS().getParameter(ParamID::kBoost);
        boost->setValueNotifyingHost(boost->convertTo0to1(1.0f));  // +6 dB ceiling
        processor.prepareToPlay(kSampleRate, 512);
        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        buffer.setSample(0, 0, 1.0f);
        buffer.setSample(1, 0, 0.5f);
        juce::MidiBuffer midi;
        processor.processBlock(buffer, midi);

        Crossover reference;
        reference.prepare(kSampleRate);
        const float expectedGain = juce::Decibels::decibelsToGain(std::min(requestedDb, 6.0f));
        std::array<float, 512> referenceOutput{};
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            const auto [low, mid, high] = reference.processSample(0, i == 0 ? 1.0f : 0.0f);
            referenceOutput[static_cast<size_t>(i)] = low + mid + high;
        }
        const float referenceRms = rmsLevel(referenceOutput.data(), buffer.getNumSamples());
        EXPECT_NEAR(rmsLevel(buffer.getReadPointer(0), buffer.getNumSamples()) / referenceRms,
                    expectedGain, 1e-5f);
        EXPECT_NEAR(rmsLevel(buffer.getReadPointer(1), buffer.getNumSamples()) / referenceRms,
                    expectedGain * 0.5f, 1e-5f);
    }
}

TEST(PluginTest, ParameterStateRoundTrip) {
    AudioPluginAudioProcessor original;
    const std::array<const char*, 4> ids{ParamID::kLow, ParamID::kMid, ParamID::kHigh, ParamID::kBoost};
    const std::array<float, 4> values{-31.4f, 6.3f, -100.0f, 2.0f};
    for (size_t i = 0; i < ids.size(); ++i) {
        auto* parameter = original.getAPVTS().getParameter(ids[i]);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(values[i]));
    }
    juce::MemoryBlock state;
    original.getStateInformation(state);
    AudioPluginAudioProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    for (size_t i = 0; i < ids.size(); ++i)
        EXPECT_NEAR(restored.getAPVTS().getRawParameterValue(ids[i])->load(), values[i], 0.001f);
}
