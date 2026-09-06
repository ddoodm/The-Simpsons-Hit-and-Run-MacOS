#!/usr/bin/env python3
"""Extract .rsd sounds from the retail .rcf archives and write them as .wav.

Usage: extract_rsd.py OUT_DIR PATTERN [PATTERN ...]
Patterns are matched against the archive path, e.g. "interactive_props/gag_shel".
Only PCM sounds are converted; anything else is reported and skipped.
"""

import glob
import os
import struct
import sys

GAME_DIR = os.environ.get("SHAR_DIR", "/Users/deinyon/Downloads/The Simpsons - Hit & Run")

# Payload starts on a sector boundary; the header occupies the first 20 bytes.
RSD_FILE_DATA_OFFSET = 2048


def key32(name):
    k = 0
    for ch in name:
        c = ord(ch)
        c = c + 32 if c < ord("a") else c
        k = ((k << 5) - k + c) & 0xFFFFFFFF
    return k


def find(archives, filename):
    h = key32(filename.replace("/", "\\"))
    for path, (data, entries) in archives.items():
        if h in entries:
            return path, data, entries[h]
    return None


def load_archives():
    archives = {}
    for path in sorted(glob.glob(os.path.join(GAME_DIR, "*.rcf"))):
        data = open(path, "rb").read()
        dir_offset = struct.unpack_from("<I", data, 44)[0]
        count = struct.unpack_from("<I", data, dir_offset)[0]
        entries = {}
        for i in range(count):
            h, off, size = struct.unpack_from("<III", data, dir_offset + 16 + 12 * i)
            entries[h] = (off, size)
        archives[path] = (data, entries)
    return archives


def write_wav(path, pcm, channels, bits, rate):
    byte_rate = rate * channels * bits // 8
    block_align = channels * bits // 8
    with open(path, "wb") as f:
        f.write(b"RIFF" + struct.pack("<I", 36 + len(pcm)) + b"WAVE")
        f.write(b"fmt " + struct.pack("<IHHIIHH", 16, 1, channels, rate, byte_rate, block_align, bits))
        f.write(b"data" + struct.pack("<I", len(pcm)))
        f.write(pcm)


def main():
    out_dir, patterns = sys.argv[1], sys.argv[2:]
    os.makedirs(out_dir, exist_ok=True)
    archives = load_archives()

    # The archives index by hash only, so candidate names come from the sound scripts.
    scripts_dir = os.path.join(os.path.dirname(__file__), "..", "src/game/sound/soundrenderer/scripts")
    names = set()
    for script in glob.glob(os.path.join(scripts_dir, "*.inl")):
        for line in open(script, errors="ignore"):
            if 'AddFilename ( "' in line:
                names.add(line.split('AddFilename ( "')[1].split('"')[0])

    for name in sorted(names):
        if not any(p in name for p in patterns):
            continue
        hit = find(archives, name)
        if hit is None:
            print(f"missing: {name}")
            continue
        archive, data, (off, size) = hit
        encoding = data[off + 4:off + 8].decode("latin1")
        channels, bits, rate = struct.unpack_from("<III", data, off + 8)
        if encoding != "PCM ":
            print(f"skipped ({encoding.strip()}): {name}")
            continue
        pcm = data[off + RSD_FILE_DATA_OFFSET:off + size]
        out_path = os.path.join(out_dir, os.path.basename(name).replace(".rsd", ".wav"))
        write_wav(out_path, pcm, channels, bits, rate)
        secs = len(pcm) / (bits // 8) / channels / rate
        print(f"{out_path}  {rate} Hz {channels}ch {bits}-bit  {secs:.2f}s  <- {os.path.basename(archive)}")


main()
