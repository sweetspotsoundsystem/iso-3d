# Iso3D - Agent Instructions

## Project Overview

3-band DJ isolator plugin (JUCE 8.0.6, C++20, GoogleTest 1.16.0).

## Build & Test

```bash
# Configure + build (debug)
cmake --preset default && cmake --build build

# Run tests
cd build && ctest

# Release build + install
cmake --preset release && cmake --build release-build
./scripts/install-plugins.sh --release
```

## Project Structure

```
iso-3d/
├── CMakeLists.txt              # Root: C++20, CPM, JUCE, GoogleTest
├── cmake/                      # cpm.cmake, Util.cmake, CompilerWarnings.cmake
├── plugin/
│   ├── CMakeLists.txt          # VST3 + AU + Standalone
│   ├── include/Iso3D/
│   │   ├── Constants.h         # Crossover freqs, param IDs, smoothing
│   │   ├── Crossover.h         # LR4 3-band crossover (stereo)
│   │   ├── PluginProcessor.h   # APVTS, processBlock, state
│   │   └── PluginEditor.h      # UI (3 knobs + boost selector)
│   └── source/
│       ├── Crossover.cpp
│       ├── PluginProcessor.cpp
│       └── PluginEditor.cpp
├── test/
│   └── source/AudioProcessorTest.cpp  # 12 GTest tests
└── scripts/
    └── install-plugins.sh
```

## Key Technical Details

### Crossover
- Uses `juce::dsp::LinkwitzRileyFilter<float>` (TPT structure), NOT manually cascaded IIR filters.
- `processSample(channel, input, outputLow, outputHigh)` guarantees LP + HP = allpass (flat magnitude).
- LP + HP = allpass means magnitude is preserved but phase differs. Tests must compare RMS levels, not sample values.

### Compiler Warnings
- Strict `-Werror` with extensive warnings. Use `size_t` for vector indexing. Remove unused functions.
- `juce::AudioProcessor::BusesProperties` needs full qualification in .cpp files.

### Testing
- JUCE Timer assertions and `ShutdownDetector` leak warnings in tests are expected (harmless).
- Crossover/gain tests use RMS/energy ratios with explicit tolerances, not strict sample equality.
- 50 Hz is ~2.3 octaves below 250 Hz crossover → ~55 dB LR4 attenuation.

## Conventions

- Namespace: `audio_plugin`
- Parameter IDs: `ParamID::kLow`, `kMid`, `kHigh`, `kBoost`
- APVTS pattern: `createParameterLayout()` static method, `getRawParameterValue()` for lock-free audio-thread access
- State save/restore via APVTS XML serialization
