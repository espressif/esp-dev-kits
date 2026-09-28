#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
"""Generate piano-like zone cue WAVs (24 kHz mono 16-bit)."""

import math
import struct
import wave
from pathlib import Path

SAMPLE_RATE = 24000
PDM_UP_SAMPLE_FS = SAMPLE_RATE // 50  # must match zone_tone.c TONE_PDM_UPSAMPLE_FS
DURATION_MS = 520
PEAK = 3000

# C4–F4, G4–C5 (zone 5 silent)
ZONE_FREQ_HZ = {
    1: 261.63,  # C4
    2: 293.66,  # D4
    3: 329.63,  # E4
    4: 349.23,  # F4
    6: 392.00,  # G4
    7: 440.00,  # A4
    8: 493.88,  # B4
    9: 523.25,  # C5
}

# (harmonic index, amplitude, decay) — higher partials fade faster (piano-like)
PIANO_PARTIALS = (
    (1.0, 1.00, 1.8),
    (2.0, 0.52, 3.2),
    (3.0, 0.28, 4.8),
    (4.0, 0.16, 6.5),
    (5.0, 0.09, 8.5),
    (6.0, 0.05, 10.5),
    (7.0, 0.03, 12.0),
)

# String stiffness → slightly sharp upper partials
INHARMONICITY_B = 0.00028


def partial_freq(base_hz: float, n: float) -> float:
    return base_hz * n * math.sqrt(1.0 + INHARMONICITY_B * n * n)


def hammer_brightness(t: float, harmonic: float) -> float:
    """Short strike adds high-frequency content, like a felt hammer."""
    if harmonic < 3.0:
        return 1.0
    return 1.0 + 0.55 * math.exp(-140.0 * t)


def global_envelope(t: float) -> float:
    """Fast hammer attack + long sustain decay."""
    attack_s = 0.0035
    if t < attack_s:
        x = t / attack_s
        return x * x * (3.0 - 2.0 * x)  # smoothstep
    return math.exp(-1.15 * (t - attack_s))


def piano_samples(freq_hz: float) -> list[int]:
    n = int(SAMPLE_RATE * DURATION_MS / 1000)
    raw: list[float] = []

    for i in range(n):
        t = i / SAMPLE_RATE
        tone = 0.0

        for harmonic, amp, decay in PIANO_PARTIALS:
            fn = partial_freq(freq_hz, harmonic)
            partial_env = math.exp(-decay * t) * hammer_brightness(t, harmonic)
            tone += amp * math.sin(2.0 * math.pi * fn * t) * partial_env

        det = 0.035 * math.sin(2.0 * math.pi * freq_hz * 1.0015 * t) * math.exp(-2.5 * t)
        tone += det
        raw.append(tone * global_envelope(t))

    peak = max(abs(x) for x in raw) or 1.0
    scale = PEAK / peak
    out: list[int] = []
    for x in raw:
        sample = int(x * scale)
        sample = max(-32767, min(32767, sample))
        out.append(sample)
    return out


def write_wav(path: Path, samples: list[int]) -> None:
    with wave.open(str(path), "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        frames = b"".join(struct.pack("<h", s) for s in samples)
        wf.writeframes(frames)


def main() -> None:
    assets = Path(__file__).resolve().parents[1] / "main" / "assets"
    assets.mkdir(parents=True, exist_ok=True)

    for zone_id, freq in sorted(ZONE_FREQ_HZ.items()):
        out_path = assets / f"zone{zone_id}.wav"
        pcm = piano_samples(freq)
        write_wav(out_path, pcm)
        peak = max(abs(s) for s in pcm)
        print(f"wrote {out_path.name}: {freq:.2f} Hz, {SAMPLE_RATE} Hz, peak={peak}, fs={PDM_UP_SAMPLE_FS}")


if __name__ == "__main__":
    main()
