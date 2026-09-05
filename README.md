# Iso3D

A 3-band DJ isolator audio plugin inspired by Electronique & Spectacle (E&S) and TEIL3 B&.

![Iso3D Screenshot](./screenshot.png)

## Features

- **3-band isolation** with crossover points at 250 Hz and 3140 Hz
- **LR4 crossover** (Linkwitz-Riley 4th order, 24 dB/oct) for clean band separation
- **Per-band gain** from full kill (-100 dB) to boost (+12 dB)
- **Configurable boost limiter** (0 dB, +6 dB, +12 dB)
- **Click-free transitions** via EMA gain smoothing (5ms time constant)
- **Zero latency** (pure IIR, sample-by-sample processing)
- **Readable controls** with band labels, editable dB values, centre detents, and one-click boost limits
- **Formats:** Standalone, VST3, AU

## MIDI Controller

You can map this plugin to a MIDI controller with 3 knobs. We created a custom controller with Jérôme Barbé. It is based on Arduino UNO R4 and I'm planning to open source the code. Stay tuned!

## Building

Requires CMake 3.22+, Ninja, and a C++20 compiler.

```bash
# Debug build
cmake --preset default
cmake --build build

# Release build
cmake --preset release
cmake --build release-build

# Run tests
cd build && ctest
```

## Installing

```bash
# Install release plugins to ~/Library/Audio/Plug-Ins/
./scripts/install-plugins.sh --release

# Install plugins from the default Debug build
./scripts/install-plugins.sh
```

Drag a knob vertically to change gain, double-click it to reset to 0 dB, or click
its value to type a gain (`KILL` gives full attenuation). Choose a maximum boost
with the three-position 0 / +6 / +12 dB selector. If a knob is set above the selected maximum,
the label beneath it shows the effective limit; the knob keeps its requested value.

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Low | -100 to +12 dB | 0 dB | Low band gain (< 250 Hz) |
| Mid | -100 to +12 dB | 0 dB | Mid band gain (250 Hz - 3140 Hz) |
| High | -100 to +12 dB | 0 dB | High band gain (> 3140 Hz) |
| Boost | 0 / +6 / +12 dB | 0 dB | Maximum boost level |

## Architecture

```
Input -> LR4(250Hz) -> LP -> AP(3140Hz) -> Low band -> gain -> ╲
                    -> HP -> LR4(3140Hz) -> LP -> Mid band  -> gain ->  sum -> Output
                                         -> HP -> High band -> gain -> ╱
```

Uses JUCE's `LinkwitzRileyFilter` (TPT structure). The low branch includes a matching
all-pass stage for the second split, so the three-band sum has a flat magnitude
response at unity gain. Its phase differs from the input.

## License

[MIT](LICENSE.md)
