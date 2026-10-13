"""Procedural dark-fantasy SFX + music for Squad Roguelike.

Writes WAV to a temp dir, then encodes to OGG Vorbis with ffmpeg into
godot/art/audio/{sfx,music}. Everything is generated from noise/oscillators
(no third-party samples), so it is original work released as CC0.
"""
import os
import shutil
import subprocess
import sys
import wave

import numpy as np

OUT = sys.argv[1]
TMP = sys.argv[2]
FFMPEG = shutil.which("ffmpeg")
SR = 32000
rng = np.random.default_rng(1234)


# ---------------------------------------------------------------- helpers
def t_axis(dur, sr=SR):
    return np.arange(int(dur * sr)) / sr


def env_adsr(n, a, d, s, r, sr=SR):
    a_n, d_n, r_n = int(a * sr), int(d * sr), int(r * sr)
    s_n = max(0, n - a_n - d_n - r_n)
    e = np.concatenate([
        np.linspace(0, 1, a_n, endpoint=False) if a_n else np.zeros(0),
        np.linspace(1, s, d_n, endpoint=False) if d_n else np.zeros(0),
        np.full(s_n, s),
        np.linspace(s, 0, r_n) if r_n else np.zeros(0),
    ])
    return np.pad(e, (0, max(0, n - len(e))))[:n]


def exp_decay(n, tau, sr=SR):
    return np.exp(-np.arange(n) / (tau * sr))


def band(sig, lo, hi, sr=SR, soft=0.15):
    """Circular FFT band-pass with soft edges (seamless for loops)."""
    spec = np.fft.rfft(sig)
    f = np.fft.rfftfreq(len(sig), 1 / sr)
    mask = np.ones_like(f)
    if lo > 0:
        mask *= 1 / (1 + np.exp(-(f - lo) / (lo * soft + 1)))
    if hi < sr / 2:
        mask *= 1 / (1 + np.exp((f - hi) / (hi * soft + 1)))
    return np.fft.irfft(spec * mask, len(sig))


def formant(sig, peaks, width=0.18, sr=SR):
    spec = np.fft.rfft(sig)
    f = np.fft.rfftfreq(len(sig), 1 / sr)
    mask = np.zeros_like(f) + 0.05
    for fc, gain in peaks:
        mask += gain * np.exp(-((f - fc) / (fc * width)) ** 2)
    return np.fft.irfft(spec * mask, len(sig))


def saw_phase(freq_curve, sr=SR):
    return np.cumsum(freq_curve) / sr


def saw(freq_curve, harmonics=40, sr=SR, rolloff=1.0):
    ph = 2 * np.pi * saw_phase(freq_curve, sr)
    out = np.zeros_like(ph)
    nyq = sr / 2
    fmax = float(np.max(freq_curve))
    for k in range(1, harmonics + 1):
        if k * fmax > nyq * 0.9:
            break
        out += np.sin(k * ph) / (k ** rolloff)
    return out


def sine(freq_curve, sr=SR):
    return np.sin(2 * np.pi * saw_phase(freq_curve, sr))


def noise(n):
    return rng.standard_normal(n)


def sweep_noise(n, f_start, f_end, q=0.5, segments=24, sr=SR):
    """Noise whose band centre glides from f_start to f_end (crossfaded bands)."""
    base = noise(n)
    out = np.zeros(n)
    centres = np.geomspace(f_start, f_end, segments)
    pos = np.linspace(0, 1, n)
    for i, fc in enumerate(centres):
        b = band(base, fc * (1 - q), fc * (1 + q), sr)
        w = np.exp(-((pos - i / (segments - 1)) * (segments - 1) / 1.2) ** 2)
        out += b * w
    return out


def bell(freq, dur, sr=SR, bright=1.0):
    t = t_axis(dur, sr)
    out = np.zeros_like(t)
    for ratio, amp, tau in [(1, 1, 1.0), (2.0, 0.5, 0.7), (2.76, 0.4, 0.5), (5.4, 0.25 * bright, 0.3), (8.9, 0.12 * bright, 0.18)]:
        out += amp * np.sin(2 * np.pi * freq * ratio * t) * np.exp(-t / (tau * dur * 0.5))
    return out


def tom(freq, dur=0.6, sr=SR):
    t = t_axis(dur, sr)
    f = freq * (1 + 1.2 * np.exp(-t / 0.03))
    body = sine(f, sr) * np.exp(-t / 0.18)
    hit = band(noise(len(t)), 80, 2500, sr) * np.exp(-t / 0.02) * 0.4
    return np.tanh((body + hit) * 1.6)


def norm(sig, peak=0.89):
    m = np.max(np.abs(sig)) or 1
    return sig / m * peak


def fade(sig, fin=0.005, fout=0.02, sr=SR):
    sig = sig.copy()
    a, b = int(fin * sr), int(fout * sr)
    if a:
        sig[:a] *= np.linspace(0, 1, a)
    if b:
        sig[-b:] *= np.linspace(1, 0, b)
    return sig


def write(name, sig, sub="sfx", sr=SR, stereo=None, quality=4):
    os.makedirs(os.path.join(OUT, sub), exist_ok=True)
    wav = os.path.join(TMP, name + ".wav")
    data = sig if stereo is None else np.stack([sig, stereo], axis=1)
    pcm = (np.clip(data, -1, 1) * 32767).astype(np.int16)
    with wave.open(wav, "wb") as w:
        w.setnchannels(1 if stereo is None else 2)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes(pcm.tobytes())
    ogg = os.path.join(OUT, sub, name + ".ogg")
    subprocess.run([FFMPEG, "-y", "-loglevel", "error", "-i", wav, "-c:a", "libvorbis", "-q:a", str(quality), ogg], check=True)
    print("wrote", ogg)


# ---------------------------------------------------------------- SFX
def swing(i):
    dur = 0.32 + 0.04 * i
    n = int(dur * SR)
    s = sweep_noise(n, 500 + 150 * i, 2600 + 300 * i, 0.45)
    e = np.sin(np.linspace(0, np.pi, n)) ** 2.2
    return fade(norm(s * e, 0.75))


def magic(i):
    dur = 0.7
    t = t_axis(dur)
    n = len(t)
    rise = sweep_noise(n, 400, 4000, 0.3) * env_adsr(n, 0.25, 0.1, 0.6, 0.3)
    f = (330 + 60 * i) * (1 + 1.5 * t / dur)
    tone = (sine(f) + 0.5 * sine(f * 1.5)) * env_adsr(n, 0.05, 0.2, 0.5, 0.35) * 0.4
    shimmer = sum(np.sin(2 * np.pi * (1200 + 400 * k + 90 * i) * t) * np.exp(-t / 0.25) for k in range(4)) * 0.08
    return fade(norm(rise * 0.6 + tone + shimmer, 0.7))


def growl(i, dur=1.0, base=70.0, pitch_drop=0.0, peaks=((380, 1.0), (850, 0.7), (2300, 0.25))):
    t = t_axis(dur)
    n = len(t)
    jitter = band(noise(n), 0, 12) * 25
    f = base * (1 + 0.12 * np.sin(2 * np.pi * 3.3 * t + i)) * (1 - pitch_drop * t / dur) + jitter
    f = np.clip(f, 25, None)
    src = saw(f, 80) * (1 + 0.6 * np.sin(2 * np.pi * (28 + 4 * i) * t))
    breath = band(noise(n), 150, 2500) * 0.6
    voice = formant(src + breath, peaks)
    voice = np.tanh(norm(voice) * 2.2)
    return fade(norm(voice * env_adsr(n, 0.08, 0.2, 0.75, 0.35), 0.8), 0.01, 0.08)


def shriek(i):
    dur = 0.65
    t = t_axis(dur)
    n = len(t)
    f = (620 + 90 * i) * (1 + 0.35 * np.sin(np.pi * t / dur)) * (1 + 0.03 * np.sin(2 * np.pi * 14 * t))
    src = saw(f, 25) + band(noise(n), 1500, 6000) * 0.5
    voice = formant(src, [(1100, 1.0), (2600, 0.8), (3800, 0.4)])
    voice = np.tanh(norm(voice) * 2.5)
    return fade(norm(voice * env_adsr(n, 0.03, 0.1, 0.8, 0.25), 0.7))


def boss_roar():
    a = growl(0, 2.6, 46, 0.25, ((260, 1.0), (620, 0.8), (1500, 0.35)))
    t = t_axis(2.6)
    sub = sine(38 * (1 - 0.2 * t / 2.6)) * env_adsr(len(t), 0.2, 0.4, 0.8, 0.8) * 0.7
    rumble = band(noise(len(t)), 20, 160) * env_adsr(len(t), 0.1, 0.3, 0.7, 1.0) * 0.6
    return fade(norm(np.tanh((a + sub + rumble) * 1.5), 0.92), 0.01, 0.2)


def monster_death(i):
    d = growl(i + 3, 0.9, 95 + 10 * i, 0.55, ((450, 1.0), (1000, 0.6)))
    t = t_axis(0.9)
    thud = tom(55, 0.9) * 0.6 * (t > 0.45)
    return fade(norm(d + np.roll(thud, 0), 0.8), 0.01, 0.1)


def horn():
    dur = 2.6
    t = t_axis(dur)
    n = len(t)
    out = np.zeros(n)
    for f0, start, amp in [(73.4, 0.0, 1.0), (110.0, 0.35, 0.7), (146.8, 0.35, 0.4)]:
        f = f0 * (1 + 0.004 * np.sin(2 * np.pi * 5 * t))
        tone = saw(f, 30, rolloff=1.3)
        s0 = int(start * SR)
        e = env_adsr(n - s0, 0.35, 0.3, 0.8, 0.9)
        out[s0:] += amp * tone[: n - s0] * e
    out = formant(out, [(350, 1.0), (700, 0.8), (1300, 0.35)], 0.3)
    out += band(noise(n), 200, 900) * env_adsr(n, 0.3, 0.3, 0.3, 0.9) * 0.05
    return fade(norm(np.tanh(norm(out) * 1.4), 0.85), 0.01, 0.3)


def wave_cleared():
    out = np.zeros(int(3.0 * SR))
    for k, f in enumerate([110.0, 164.8, 220.0, 261.6]):
        b = bell(f, 3.0, bright=0.7) * (0.9 - 0.12 * k)
        out += np.roll(b, int(k * 0.09 * SR)) * (np.arange(len(out)) >= int(k * 0.09 * SR))
    return fade(norm(out, 0.75), 0.002, 0.4)


def level_up():
    dur = 1.6
    out = np.zeros(int(dur * SR))
    for k, f in enumerate([220.0, 261.6, 329.6, 440.0, 523.3]):
        start = int(k * 0.08 * SR)
        b = bell(f * 2, dur - k * 0.08, bright=1.2)
        out[start:start + len(b)] += b[: len(out) - start] * 0.6
    t = t_axis(dur)
    out += sweep_noise(len(t), 1500, 7000, 0.25) * env_adsr(len(t), 0.4, 0.3, 0.3, 0.6) * 0.25
    return fade(norm(out, 0.75), 0.002, 0.3)


def card_pick():
    dur = 0.55
    t = t_axis(dur)
    n = len(t)
    swish = sweep_noise(n, 2500, 900, 0.4) * np.exp(-t / 0.08) * 0.5
    chime = (bell(880, dur, bright=0.6) + 0.6 * bell(1318.5, dur, bright=0.5)) * 0.4
    return fade(norm(swish + chime, 0.7))


def stinger(victory):
    dur = 4.0
    t = t_axis(dur)
    n = len(t)
    chord = [73.4, 110.0, 146.8, 185.0] if victory else [73.4, 87.3, 110.0, 103.8]
    out = np.zeros(n)
    for k, f0 in enumerate(chord):
        glide = 1.0 if victory else (1 - 0.06 * t / dur)
        f = f0 * glide * (1 + 0.003 * np.sin(2 * np.pi * (4.5 + k) * t))
        out += saw(f, 30, rolloff=1.4) * (0.9 - 0.1 * k)
    out = formant(out, [(300, 1.0), (650, 0.8), (1200, 0.3)], 0.35)
    out *= env_adsr(n, 0.6 if victory else 0.25, 0.5, 0.7, 1.8)
    out += tom(45, dur) * 0.8
    if victory:
        out += bell(440, dur, bright=0.8) * 0.15
    return fade(norm(np.tanh(norm(out) * 1.3), 0.85), 0.01, 0.6)


def hurt(i):
    dur = 0.35
    t = t_axis(dur)
    n = len(t)
    f = (180 + 30 * i) * (1 - 0.4 * t / dur)
    v = formant(saw(f, 30) + band(noise(n), 300, 3000) * 0.4, [(600, 1.0), (1200, 0.6)])
    return fade(norm(np.tanh(norm(v) * 2) * env_adsr(n, 0.01, 0.08, 0.6, 0.2), 0.65))


def player_death():
    dur = 2.2
    t = t_axis(dur)
    n = len(t)
    f = 140 * (1 - 0.55 * t / dur)
    v = formant(saw(f, 40), [(500, 1.0), (1000, 0.5)]) * env_adsr(n, 0.05, 0.4, 0.5, 1.2)
    toll = bell(65.4, dur, bright=0.4) * 0.8
    return fade(norm(np.tanh(norm(v) * 1.5) * 0.7 + toll, 0.8), 0.01, 0.4)


def ui_hover(i):
    dur = 0.09
    t = t_axis(dur)
    tone = np.sin(2 * np.pi * (1900 + 250 * i) * t) * np.exp(-t / 0.018)
    tick = band(noise(len(t)), 2500, 8000) * np.exp(-t / 0.006) * 0.4
    return fade(norm(tone * 0.5 + tick, 0.45), 0.001, 0.01)


def ui_click(i):
    dur = 0.22
    t = t_axis(dur)
    n = len(t)
    metal = sum(np.sin(2 * np.pi * f * t) * np.exp(-t / tau) for f, tau in
                [(620 + 40 * i, 0.08), (1710 + 60 * i, 0.05), (3120, 0.03), (4440, 0.02)])
    hit = band(noise(n), 1000, 7000) * np.exp(-t / 0.004)
    thump = np.sin(2 * np.pi * 130 * t) * np.exp(-t / 0.03)
    return fade(norm(metal * 0.5 + hit * 0.6 + thump * 0.5, 0.7), 0.0005, 0.02)


def ui_back():
    dur = 0.25
    t = t_axis(dur)
    f = 900 * (1 - 0.45 * t / dur)
    s = sine(f) * np.exp(-t / 0.06) + band(noise(len(t)), 800, 5000) * np.exp(-t / 0.01) * 0.4
    return fade(norm(s, 0.55))


# ---------------------------------------------------------------- music
MSR = 32000


def music_menu():
    dur = 64.0
    t = t_axis(dur, MSR)
    n = len(t)
    out_l = np.zeros(n)
    out_r = np.zeros(n)
    # Drone: D1/D2/A2 detuned saws, slow breathing (LFO periods divide the loop).
    for f0, amp in [(36.7, 0.6), (73.4, 0.5), (110.0, 0.3), (146.8, 0.12)]:
        for det, chan in [(-0.12, 0), (0.12, 1)]:
            ph_lfo = 0.3 + 0.7 * (0.5 + 0.5 * np.sin(2 * np.pi * t / 16 + f0))
            tone = saw(np.full(n, f0 + det), 50, MSR, 1.4) * amp * ph_lfo
            if chan == 0:
                out_l += tone
            else:
                out_r += tone
    out_l = band(out_l, 20, 700, MSR)
    out_r = band(out_r, 20, 700, MSR)
    # Wind: circular band noise -> seamless.
    wind_l = band(noise(n), 200, 900, MSR) * (0.4 + 0.6 * (0.5 + 0.5 * np.sin(2 * np.pi * t / 32)))
    wind_r = band(noise(n), 200, 900, MSR) * (0.4 + 0.6 * (0.5 + 0.5 * np.sin(2 * np.pi * t / 32 + 2)))
    out_l += wind_l * 0.25
    out_r += wind_r * 0.25
    # Distant bell tolls; tails wrap around the loop point.
    for start, f in [(2.0, 146.8), (18.0, 130.8), (34.0, 146.8), (50.0, 110.0)]:
        b = bell(f, 10.0, MSR, 0.5) * 0.35
        idx = (np.arange(len(b)) + int(start * MSR)) % n
        np.add.at(out_l, idx, b * 0.8)
        np.add.at(out_r, idx, b)
    # Sparse high minor melody (dulcimer-like plucks).
    notes = [(8, 293.7), (9.5, 349.2), (11, 329.6), (24, 293.7), (26, 261.6), (40, 293.7), (41.5, 349.2), (43, 440.0), (44.5, 415.3), (56, 293.7)]
    for start, f in notes:
        pl = bell(f, 3.0, MSR, 0.9) * 0.12
        idx = (np.arange(len(pl)) + int(start * MSR)) % n
        np.add.at(out_l, idx, pl)
        np.add.at(out_r, idx, pl * 0.7)
    m = max(np.max(np.abs(out_l)), np.max(np.abs(out_r)))
    return out_l / m * 0.8, out_r / m * 0.8


def drum_pattern(n, bpm, pattern, sr=MSR, pitch=60.0, gain=1.0):
    out = np.zeros(n)
    beat = 60.0 / bpm
    total_beats = int(n / sr / beat)
    for b in range(total_beats):
        for step, (hit, acc) in enumerate(pattern):
            if not hit:
                continue
            pos = int((b + step / len(pattern)) * beat * sr)
            d = tom(pitch * (1.3 if acc < 0 else 1.0), 0.6, sr) * abs(acc) * gain
            idx = (np.arange(len(d)) + pos) % n
            np.add.at(out, idx, d)
    return out


def ostinato(n, bpm, notes, sr=MSR, octave=1.0):
    out = np.zeros(n)
    beat = 60.0 / bpm
    step = beat / 2
    count = int(n / sr / step)
    for k in range(count):
        f = notes[k % len(notes)] * octave
        dur = step * 0.95
        tt = t_axis(dur, sr)
        tone = saw(np.full(len(tt), f), 30, sr, 1.3) * env_adsr(len(tt), 0.01, 0.08, 0.5, 0.08, sr)
        idx = (np.arange(len(tone)) + int(k * step * sr)) % n
        np.add.at(out, idx, tone)
    return band(out, 40, 1200, sr)


def music_combat(boss=False):
    bpm = 132.0 if boss else 100.0
    bars = 16 if boss else 12
    dur = bars * 4 * 60.0 / bpm
    t = t_axis(dur, MSR)
    n = len(t)
    drone = np.zeros(n)
    for f0, amp in [(36.7, 0.6), (55.0, 0.4), (73.4, 0.4)] + ([(51.9, 0.35)] if boss else []):
        drone += saw(np.full(n, f0), 40, MSR, 1.4) * amp
    drone = band(drone, 20, 500, MSR) * (0.6 + 0.4 * np.sin(2 * np.pi * t / dur * 2) ** 2)
    main = [(1, 1.0), (0, 0), (0, 0), (1, 0.5), (1, 0.8), (0, 0), (1, 0.4), (0, 0)]
    fill = [(1, 1.0), (1, 0.4), (1, 0.6), (1, 0.4), (1, 0.9), (0, 0), (1, -0.7), (1, -0.6)]
    drums = drum_pattern(n, bpm, fill if boss else main, pitch=58.0)
    taiko = drum_pattern(n, bpm, [(1, 1.0), (0, 0), (0, 0), (0, 0)], pitch=40.0, gain=1.2)
    notes = [73.4, 73.4, 87.3, 73.4, 77.8, 73.4, 65.4, 69.3] if boss else [73.4, 73.4, 87.3, 82.4, 73.4, 73.4, 65.4, 69.3]
    ost = ostinato(n, bpm, notes, octave=2.0) * (0.55 if boss else 0.4)
    choir = np.zeros(n)
    if boss:
        for f0 in [146.8, 207.7, 220.0]:   # tritone + fifth dissonance
            choir += saw(np.full(n, f0) * (1 + 0.004 * np.sin(2 * np.pi * 5 * t)), 30, MSR, 1.2)
        choir = formant(choir, [(500, 1.0), (850, 0.8), (2500, 0.2)], 0.25, MSR) * (0.5 + 0.5 * np.sin(2 * np.pi * t / dur * 4) ** 2)
    wind = band(noise(n), 300, 1500, MSR) * 0.12
    mix_l = drone * 0.5 + drums * 0.75 + taiko * 0.6 + ost + choir * 0.25 + wind
    mix_r = drone * 0.5 + np.roll(drums, 120) * 0.7 + taiko * 0.6 + np.roll(ost, 300) + choir * 0.3 + band(noise(n), 300, 1500, MSR) * 0.12
    m = max(np.max(np.abs(mix_l)), np.max(np.abs(mix_r)))
    return np.tanh(mix_l / m * 1.2) * 0.8, np.tanh(mix_r / m * 1.2) * 0.8


def main():
    os.makedirs(TMP, exist_ok=True)
    for i in range(3):
        write(f"swing_{i + 1}", swing(i))
        write(f"monster_growl_{i + 1}", growl(i, 1.0 + 0.2 * i, 62 + 12 * i))
        write(f"monster_death_{i + 1}", monster_death(i))
        write(f"hurt_{i + 1}", hurt(i))
        write(f"ui_hover_{i + 1}", ui_hover(i))
        write(f"ui_click_{i + 1}", ui_click(i))
    for i in range(2):
        write(f"magic_{i + 1}", magic(i))
        write(f"monster_shriek_{i + 1}", shriek(i))
    write("boss_roar_1", boss_roar())
    write("wave_start_1", horn())
    write("wave_cleared_1", wave_cleared())
    write("level_up_1", level_up())
    write("card_pick_1", card_pick())
    write("victory_1", stinger(True))
    write("defeat_1", stinger(False))
    write("player_death_1", player_death())
    write("ui_back_1", ui_back())
    left, right = music_menu()
    write("music_menu", left, "music", MSR, right, 3)
    left, right = music_combat(False)
    write("music_combat", left, "music", MSR, right, 3)
    left, right = music_combat(True)
    write("music_boss", left, "music", MSR, right, 3)


if __name__ == "__main__":
    main()
