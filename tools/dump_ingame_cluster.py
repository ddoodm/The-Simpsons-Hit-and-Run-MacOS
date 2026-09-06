#!/usr/bin/env python3
"""List the SC_INGAME sound cluster and locate each file in the retail .rcf archives."""

import glob
import os
import re
import struct
import sys

GAME_DIR = sys.argv[1] if len(sys.argv) > 1 else "/Users/deinyon/Downloads/The Simpsons - Hit & Run"
SCRIPTS = os.path.join(os.path.dirname(__file__), "..", "src/game/sound/soundrenderer/scripts")

# SC_INGAME is set in effects.cpp (positionalsounds.inl) and in the per-language
# script files (interactive_props*.inl).
CLUSTER_FILES = ["positionalsounds.inl", "interactive_props.inl"]


def key32(name):
    """radMakeCaseInsensitiveKey32: the hash the cement archives index by."""
    k = 0
    for ch in name:
        c = ord(ch)
        c = c + 32 if c < ord("a") else c
        k = ((k << 5) - k + c) & 0xFFFFFFFF
    return k


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
        archives[os.path.basename(path)] = (data, entries)
    return archives


def lookup(archives, filename):
    h = key32(filename.replace("/", "\\"))
    for name, (data, entries) in archives.items():
        if h in entries:
            off, size = entries[h]
            enc = data[off + 4:off + 8].decode("latin1").strip()
            channels, bits, rate = struct.unpack_from("<III", data, off + 8)
            seconds = (size - 32) / (bits / 8) / channels / rate
            return name, enc, channels, bits, rate, size, seconds
    return None


def parse_resources(path):
    """Yield (resource name, [filenames]) in declaration order."""
    text = open(path, errors="ignore").read()
    for block in text.split("Create<daSoundResourceData>")[1:]:
        name = re.match(r'\s*\("([^"]+)"\)', block)
        files = re.findall(r'AddFilename \( "([^"]+)"', block)
        if name:
            yield name.group(1), files


archives = load_archives()
rows = []
for script in CLUSTER_FILES:
    path = os.path.join(SCRIPTS, script)
    for resource, files in parse_resources(path):
        for filename in files:
            info = lookup(archives, filename)
            rows.append((resource, filename, info))

print(f"SC_INGAME cluster: {len(rows)} entries\n")
header = f"{'resource':<20} {'archive':<12} {'enc':<5} {'rate':>6} {'ch':>3} {'bytes':>8} {'secs':>6}  file"
print(header)
print("-" * len(header))
for resource, filename, info in rows:
    if info is None:
        print(f"{resource:<20} {'MISSING':<12} {'':<5} {'':>6} {'':>3} {'':>8} {'':>6}  {filename}")
        continue
    archive, enc, channels, bits, rate, size, seconds = info
    print(f"{resource:<20} {archive:<12} {enc:<5} {rate:>6} {channels:>3} {size:>8} {seconds:>6.2f}  {filename}")

archive_tally = {}
for _, _, info in rows:
    archive_tally[info[0] if info else "MISSING"] = archive_tally.get(info[0] if info else "MISSING", 0) + 1
print("\nby archive:", archive_tally)
