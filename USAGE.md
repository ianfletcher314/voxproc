# VoxProc - Usage Guide

**Vocal Processing Plugin with Compression, De-Essing, and EQ**

VoxProc is a dedicated vocal channel strip designed to handle the most common vocal processing tasks in one plugin. Featuring a musical compressor, split-band de-esser, and comprehensive 5-band EQ with high-pass filter, it's your one-stop solution for professional vocal sounds.

---

## Use Cases in Modern Rock Production

### Drum Bus Processing

Not recommended for drums - use Bus Glue instead. VoxProc is optimized for vocal frequencies.

### Guitar Bus / Individual Tracks

Not the intended use, but VoxProc's EQ and compression can work on acoustic guitars.

**Acoustic Guitar (if needed):**
- HPF: 80 Hz, 12 dB/oct
- Compressor: Threshold -15 dB, Ratio 3:1, Attack 20ms, Release 150ms
- De-Esser: Bypass (or use on string squeaks at 3-4 kHz)
- Low-Mid: Cut 300-400 Hz by 2-3 dB (reduce mud)
- High Shelf: Boost 8 kHz by 2-3 dB (add air)

### Bass Guitar

Not recommended - use NeveStrip or Bus Glue for bass processing.

### Vocals

This is VoxProc's specialty. Here are settings for various vocal styles in rock production.

**Modern Rock Lead Vocal:**
- Input Gain: Match to -18 dBFS average
- HPF: 80-100 Hz, 24 dB/oct
- Compressor: Threshold -18 dB, Ratio 4:1, Attack 10ms, Release 100ms, Knee 6 dB
- De-Esser: Frequency 6 kHz, Threshold -18 dB, Range 6 dB, Mode Split-Band
- Low Shelf: 200 Hz, -2 dB (reduce mud)
- Low-Mid: 400 Hz, Q 1.5, -2 dB (reduce boxiness)
- Mid: 2.5-3 kHz, Q 1.0, +2 dB (presence)
- High-Mid: 5 kHz, Q 1.0, +1 dB (clarity)
- High Shelf: 10 kHz, +2 dB (air)

**Aggressive Rock Vocal:**
- HPF: 120 Hz, 24 dB/oct
- Compressor: Threshold -20 dB, Ratio 6:1, Attack 5ms, Release 80ms
- De-Esser: Frequency 5 kHz, Threshold -15 dB, Range 8 dB
- Low-Mid: 350 Hz, -3 dB
- Mid: 3 kHz, +3 dB (aggressive presence)
- High-Mid: 4.5 kHz, +2 dB
- High Shelf: 8 kHz, +1 dB

**Soft/Ballad Vocal:**
- HPF: 60-80 Hz, 12 dB/oct
- Compressor: Threshold -15 dB, Ratio 3:1, Attack 15ms, Release 150ms, Knee 8 dB
- De-Esser: Frequency 7 kHz, Threshold -20 dB, Range 4 dB
- Low Shelf: 150 Hz, +1 dB (warmth)
- Mid: 2 kHz, +1 dB (gentle presence)
- High Shelf: 12 kHz, +3 dB (air)

**Screaming/Harsh Vocals:**
- HPF: 150 Hz, 24 dB/oct
- Compressor: Threshold -25 dB, Ratio 8:1, Attack 2ms, Release 50ms
- De-Esser: Frequency 4-5 kHz, Threshold -12 dB, Range 10 dB, Mode Wideband
- Low-Mid: 400-500 Hz, -4 dB
- Mid: 2.5 kHz, +1-2 dB
- High-Mid: 6 kHz, -2 dB (tame harshness)

**Backing Vocals:**
- HPF: 150-200 Hz, 24 dB/oct (clear low end for lead)
- Compressor: Threshold -20 dB, Ratio 4:1, Attack 10ms, Release Auto
- De-Esser: Same as lead or slightly more aggressive
- Low Shelf: 250 Hz, -3 dB
- Mid: 3 kHz, -1 dB (tuck behind lead)
- High Shelf: 10 kHz, -1 dB

**Double-Tracked Vocals:**
- HPF: 120 Hz, 24 dB/oct
- Compression: Heavier than lead (Ratio 5:1)
- De-Esser: More aggressive
- Mid: 3 kHz, -2 dB (avoid masking lead)

### Mix Bus / Mastering

Not intended for mix bus - use MasterBus or Automaster.

---

## Recommended Settings

### Quick Reference Table

| Vocal Type | HPF | Comp Ratio | Comp Attack | De-Ess Freq | De-Ess Range |
|-----------|-----|------------|-------------|-------------|--------------|
| Rock Lead | 80-100 Hz | 4:1 | 10 ms | 6 kHz | 6 dB |
| Aggressive | 120 Hz | 6:1 | 5 ms | 5 kHz | 8 dB |
| Ballad | 60-80 Hz | 3:1 | 15 ms | 7 kHz | 4 dB |
| Screaming | 150 Hz | 8:1 | 2 ms | 4-5 kHz | 10 dB |
| Backing | 150-200 Hz | 4:1 | 10 ms | 6 kHz | 6-8 dB |

### Signal Flow (Internal)

VoxProc processes in this order:
1. Input Gain
2. High-Pass Filter
3. EQ
4. Compressor
5. De-Esser
6. Output Gain

### De-Esser Mode Guide

- **Split-Band**: Only affects the sibilant frequency range - more natural, preserves overall brightness
- **Wideband**: Reduces entire signal when sibilance is detected - more aggressive, can sound more obvious

### De-Esser Frequency Guide

- **Male Vocals**: 4-6 kHz typical
- **Female Vocals**: 6-8 kHz typical
- **Bright/Harsh**: Try lower frequencies (4-5 kHz)
- **Dull/Dark**: Try higher frequencies (7-8 kHz) or less reduction

Use the **Listen** mode to solo the sidechain and find the exact frequency.

---

## Signal Flow Tips

### Where to Place VoxProc

1. **First Insert**: VoxProc should typically be the first or second plugin on your vocal chain

2. **After Pitch Correction**: If using pitch correction, place it before VoxProc

3. **Before Saturation/Effects**: Process clean vocal first, then add character

### Gain Staging

- Set Input Gain so the compressor sees peaks around -18 to -12 dBFS
- Aim for 4-8 dB of compression on loud phrases
- Use Output Gain to match bypassed level
- Adjust Makeup Gain to compensate for compression

---

## Combining with Other Plugins

### Complete Lead Vocal Chain
1. **Pitch Correction** (if needed)
2. **VoxProc** - main processing
3. **TapeWarm** (subtle) - analog warmth
4. **Reverb Send** - space
5. **Delay Send** - depth

### Backing Vocal Chain
1. **VoxProc** - processing with more aggressive HPF
2. **StereoImager** - spread L/R
3. **Bus Glue** (on BV bus) - glue stacks together

### Vocal Production Order
1. **Record clean** - no processing
2. **VoxProc** - tonal shaping and dynamics
3. **Creative effects** - distortion, modulation (separate plugins)
4. **Time-based effects** - reverb, delay (sends)

---

## Quick Start Guide

**Get a professional rock vocal sound in 60 seconds:**

1. Insert VoxProc on your vocal track
2. Set **Input Gain** to bring peaks to around -12 dBFS on the input meter
3. Set **HPF** to 100 Hz, 24 dB/oct
4. Enable **Compressor**:
   - Threshold: -18 dB
   - Ratio: 4:1
   - Attack: 10 ms
   - Release: 100 ms (or enable Auto Release)
   - Knee: 6 dB
5. Set **Comp Makeup** to compensate (typically +3-6 dB)
6. Enable **De-Esser**:
   - Frequency: 6000 Hz
   - Threshold: -18 dB
   - Range: 6 dB
   - Mode: Split-Band
7. Use **Listen** mode to verify de-esser is targeting sibilance
8. Set EQ:
   - Low Shelf: 200 Hz, -2 dB
   - Low-Mid: 400 Hz, Q 1.5, -2 dB
   - Mid: 2.8 kHz, Q 1.0, +2 dB
   - High-Mid: 5 kHz, Q 1.0, +1 dB
   - High Shelf: 10 kHz, +2 dB
9. Adjust **Output Gain** to match bypass level
10. A/B with bypass and tweak to taste

**Fix a problematic vocal in 30 seconds:**

1. **Too much low end rumble?** Increase HPF to 120-150 Hz
2. **Too muddy?** Cut Low-Mid (300-500 Hz) by 3-4 dB
3. **Too harsh/sibilant?** Lower de-esser threshold, increase range
4. **No presence?** Boost Mid (2.5-3.5 kHz) by 2-3 dB
5. **Too dynamic?** Lower comp threshold or increase ratio
6. **Too compressed?** Raise threshold, reduce ratio, slow attack
