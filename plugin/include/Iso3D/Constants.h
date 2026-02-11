#pragma once

namespace audio_plugin {

constexpr int kNumChannels = 2;
constexpr int kNumBands = 3;

// Crossover frequencies (TEIL3-style)
constexpr float kLowMidCrossoverHz = 250.0f;
constexpr float kMidHighCrossoverHz = 3140.0f;  // pi kHz

// Gain smoothing: alpha = 1 - exp(-1 / (tau * sr)), computed at prepare()
constexpr float kGainSmoothTimeSec = 0.005f;  // 5ms time constant
constexpr float kKillThresholdDb = -100.0f;
constexpr float kUnityDeadZoneDb = 0.5f;  // snap to 0 dB within +/-0.5 dB
constexpr float kBoostLevels[] = {0.0f, 6.0f, 12.0f};

// Parameter IDs
namespace ParamID {
inline constexpr const char* kLow = "low";
inline constexpr const char* kMid = "mid";
inline constexpr const char* kHigh = "high";
inline constexpr const char* kBoost = "boost";
}  // namespace ParamID

}  // namespace audio_plugin
