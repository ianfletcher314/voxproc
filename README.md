# VoxProc - Vocal Processing Plugin

A dedicated vocal processing plugin combining compression, de-essing, and EQ in a streamlined interface optimized for vocal tracks.

## Overview

VoxProc is designed to be the one-stop solution for vocal processing, combining the essential tools every vocal needs in a single, intuitive interface. Built with JUCE for AU/VST3 compatibility.

## Features

### 1. Compressor Section
- **Threshold**: -60dB to 0dB
- **Ratio**: 1:1 to 20:1 (with soft-knee option)
- **Attack**: 0.1ms to 100ms
- **Release**: 10ms to 1000ms (with auto-release option)
- **Makeup Gain**: 0dB to 24dB
- **Gain Reduction Meter**: Visual feedback
- **Vocal-optimized presets**: Gentle, Broadcast, Aggressive

### 2. De-Esser Section
- **Frequency**: 2kHz to 12kHz (target sibilance range)
- **Threshold**: Sensitivity control
- **Range**: Maximum reduction amount (0-12dB)
- **Mode**: Split-band vs Wideband
- **Listen Mode**: Solo the sibilance detection band
- **Visual indicator**: Shows when de-essing is active

### 3. EQ Section
- **High Pass Filter**: 20Hz to 400Hz, 12/24dB slope
- **Low Shelf**: Adjustable frequency, +/-12dB
- **Low-Mid Band**: Parametric, 100Hz-1kHz (mud removal)
- **Mid Band**: Parametric, 500Hz-4kHz (presence/body)
- **High-Mid Band**: Parametric, 2kHz-8kHz (air/clarity)
- **High Shelf**: Adjustable frequency, +/-12dB
- **EQ Visualization**: Real-time frequency response curve

### 4. Additional Features
- **Input/Output Gain** with metering
- **A/B Comparison** toggle
- **Signal flow routing**: Choose processing order
- **Preset system**: Save/load vocal chains
- **Sidechain input**: For ducking or external keying

## Signal Flow

```
Input -> HPF -> EQ -> Compressor -> De-Esser -> Output
              (configurable order)
```

## UI Design

```
+------------------------------------------------------------------+
|  VOXPROC                                    [A/B] [Preset v]     |
+------------------------------------------------------------------+
|                                                                   |
|  +-- COMPRESSOR --+  +-- DE-ESSER --+  +-------- EQ ---------+   |
|  |                |  |              |  |                      |   |
|  |  [THRESH]      |  |  [FREQ]      |  |  ~~~~/\~~~~~/\~~~   |   |
|  |  [RATIO ]      |  |  [THRESH]    |  |  (frequency curve)   |   |
|  |  [ATTACK]      |  |  [RANGE ]    |  |                      |   |
|  |  [RELEAS]      |  |  [MODE  v]   |  |  [HPF] [LS] [LM]    |   |
|  |  [MAKEUP]      |  |  [LISTEN]    |  |  [MD] [HM] [HS]     |   |
|  |                |  |              |  |                      |   |
|  |  [GR METER]    |  |  [ACTIVE]    |  |  [GAIN] [Q] [FREQ]  |   |
|  |  [BYPASS]      |  |  [BYPASS]    |  |  [BYPASS]           |   |
|  +----------------+  +--------------+  +----------------------+   |
|                                                                   |
|  [INPUT]  ========================================  [OUTPUT]      |
+------------------------------------------------------------------+
```

## Implementation Plan

### Phase 1: Project Setup
- [ ] Create JUCE project with Projucer
- [ ] Set up AU/VST3 build targets
- [ ] Create basic plugin shell with parameter layout
- [ ] Set up GitHub repo and CI/CD

### Phase 2: DSP Implementation
- [ ] Implement compressor DSP
  - [ ] Envelope follower (RMS/Peak detection)
  - [ ] Gain computer with soft knee
  - [ ] Gain smoothing
  - [ ] Auto-release algorithm
- [ ] Implement de-esser DSP
  - [ ] Bandpass filter for detection
  - [ ] Split-band processing mode
  - [ ] Wideband ducking mode
  - [ ] Smooth gain reduction
- [ ] Implement EQ DSP
  - [ ] Biquad filter implementation
  - [ ] High-pass filter with variable slope
  - [ ] Parametric bands
  - [ ] Shelf filters

### Phase 3: UI Development
- [ ] Design dark/professional UI theme
- [ ] Create custom knob components
- [ ] Implement gain reduction meters
- [ ] Build EQ visualization
  - [ ] Frequency response curve
  - [ ] Draggable EQ points
- [ ] Add de-esser activity indicator
- [ ] Input/output metering

### Phase 4: Features & Polish
- [ ] Implement A/B comparison
- [ ] Add preset system
- [ ] Signal flow routing options
- [ ] Listen mode for de-esser
- [ ] Parameter smoothing
- [ ] CPU optimization

### Phase 5: Testing & Release
- [ ] Test in multiple DAWs
- [ ] Verify automation works
- [ ] Create factory presets
- [ ] Build installers
- [ ] Documentation

## Technical Specifications

- **Sample Rates**: 44.1kHz, 48kHz, 88.2kHz, 96kHz, 192kHz
- **Latency**: Minimal (< 1ms, zero-latency modes available)
- **Formats**: AU (macOS), VST3 (macOS/Windows)
- **CPU**: Optimized SIMD processing

## Dependencies

- JUCE Framework 7.x
- C++17 or later

## Building

```bash
# macOS
cd Builds/MacOSX
xcodebuild -project VoxProc.xcodeproj -configuration Release

# Windows
cd Builds\VisualStudio2022
msbuild VoxProc.sln /p:Configuration=Release
```

## License

MIT License - See LICENSE file

## Author

Created with JUCE Framework
