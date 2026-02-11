#pragma once

#include <juce_dsp/juce_dsp.h>

#include "Constants.h"

namespace audio_plugin {

struct BandSamples {
    float low;
    float mid;
    float high;
};

// LR4 (Linkwitz-Riley 4th order, 24 dB/oct) 3-band crossover.
//
// Uses JUCE's LinkwitzRileyFilter (TPT structure) which guarantees:
//   LP(f) + HP(f) = allpass with unit magnitude
//
// Topology:
//   Input -> LR4(lowMid) -> LP -> Low band
//                        -> HP -> LR4(midHigh) -> LP -> Mid band
//                                               -> HP -> High band
//
// Perfect reconstruction: Low + Mid + High = Input
class Crossover {
public:
    void prepare(double sampleRate);
    BandSamples processSample(int channel, float input);

private:
    // Crossover 1: low/mid split at 250 Hz
    juce::dsp::LinkwitzRileyFilter<float> lowMidSplit_;

    // Crossover 2: mid/high split at 3140 Hz (applied to HP output of split 1)
    juce::dsp::LinkwitzRileyFilter<float> midHighSplit_;
};

}  // namespace audio_plugin
