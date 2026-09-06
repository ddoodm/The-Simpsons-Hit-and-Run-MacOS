# The Simpsons: Hit & Run — macOS

A macOS port of _The Simpsons: Hit & Run_, using CMake in place of the original Visual Studio solution.

The game runs natively on Apple silicon against a retail PC copy of the game data. It uses macOS's legacy
OpenGL 2.1 profile, SDL2 for windowing and input, OpenAL Soft for audio, and FFmpeg for the movies.

## Upstream & Credits

This is a downstream project based on [3UR/Simpsons-Hit-Run](https://github.com/3UR/Simpsons-Hit-Run), which did
the heavy lifting of getting the game onto x64, C++20, vcpkg and OpenGL. All credit for that work goes to
[3UR](https://github.com/3UR) and the contributors to that repository.

This repository has been modified from that source, starting September 2026, to build with CMake and run on macOS.
It is not affiliated with or endorsed by the upstream project, and changes made here are not submitted back to it.
Licensed under GPL-3.0, the same as upstream — see [LICENSE](LICENSE).

## Building and Running on macOS

### What you need

- An Apple silicon Mac. Developed and tested on macOS 26 / arm64.
- Xcode or the Xcode Command Line Tools, for Apple Clang. `xcode-select --install`
- [Homebrew](https://brew.sh), for the libraries.
- A retail PC install of the game. The data is **not** included here — see [Game data](#game-data).

### 1. Install the dependencies

```sh
brew install cmake pkgconf sdl2 openal-soft ffmpeg libpng
```

CMake finds these through Homebrew automatically, including the keg-only ones. vcpkg is not needed on macOS,
so there is no need to clone the submodule. If you do want vcpkg, set `VCPKG_ROOT` and the root
`CMakeLists.txt` will pick up its toolchain instead.

### 2. Get the source

```sh
git clone https://github.com/ddoodm/The-Simpsons-Hit-and-Run-MacOS.git
cd The-Simpsons-Hit-and-Run-MacOS
```

### 3. Configure and build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(sysctl -n hw.ncpu)
```

The configure step prints where each dependency came from, which is the quickest way to spot a missing one.
The binary lands at `build/bin/SRR2`.

Build types map to the old Visual Studio configurations: `Debug`, `Release`, and `RelWithDebInfo` in place of
`Tune`. `Debug` is the configuration the port has actually been exercised in. It keeps asserts live, which is
useful while the port is still young but does mean a failed assert stops the game (see
[Known issues](#known-issues-on-macos)).

### 4. Run

```sh
cd "/path/to/The Simpsons - Hit & Run"
/path/to/The-Simpsons-Hit-and-Run-MacOS/build/bin/SRR2
```

### Known issues on macOS

- **No force feedback.** The effect classes are written directly against DirectInput's `DIEFFECT`, so they
  are gated behind `RAD_FORCE_FEEDBACK` and left out.
- **Memory corruption**, inherited from upstream.

## What This Fork Changes

Grouped roughly in the order the work happened. Each commit message carries the detail.

**Build system.** CMake replaces `SRR2.sln` for non-Windows targets, covering all twelve static libraries and
the game executable. Source lists are generated from the `.vcxproj` files by
[tools/gen_cmake_sources.py](tools/gen_cmake_sources.py) so they stay faithful to the Visual Studio build.
Dependencies resolve from either Homebrew or vcpkg behind `srr2::*` targets, so no library CMakeLists cares
which supplied them. `compile_commands.json` is exported for clangd.

**`RAD_MACOS` platform.** A new platform define threaded through radcore's validity and declaration gates,
with `radPlatform` built on the SDL path instead of the Win32 HWND accessors. The PC-behaviour gates across
camera, input, save data and the vehicle controller include macOS, since it is a desktop target.

**Apple Clang.** All twelve libraries compile: precompiled headers no longer leak `windows.h` into every
translation unit, `radLinkedClass` static member specialisations are declared ahead of the inline members that
use them, and a handful of MSVC-only includes and varargs assumptions are fixed.

**OpenGL 2.1.** macOS ships a legacy 2.1 profile with the fixed-function pipeline the renderer needs, so the
driver runs almost unmodified. `OpenGL.framework` exports every entry point directly, so glad is skipped;
`KHR_debug` does not exist in that profile, so the debug callback compiles out; and `PickPixelFormat` returns
S3TC enums instead of BPTC to match the upload path. `display_win32` is renamed `display_sdl`, since it held
no Win32 code.

**Platform layer.** `win32platform.cpp` contained exactly three Win32 calls in 1935 lines, so rather than
cloning it, the implementation is shared and renamed to `sdlplatform` (`Win32Platform` → `SdlPlatform`), with
the single-instance lock left Win32-only. `win32main.cpp` becomes a portable `main.cpp`.

**Input.** macOS follows the PC path rather than the console one, so keyboard, mouse look and remappable
controls all work. radcore's SDL backend publishes `Keyboard0` and `Mouse0` devices alongside pads, reporting
the same DirectInput codes the existing PC device wrappers speak.

**Files.** macOS had no drive at all, so nothing could be opened. It now gets a single drive that resolves
against the working directory. Game data paths are written with backslashes, which are an ordinary filename
character here rather than a separator, so the drive converts them in one place instead of at hundreds of call
sites.

**Memory.** Replacing global `operator new` means every loaded system library allocates through the game's
heaps, which were not built to serve them. Allocations from dyld's library initializers now come from `malloc`
behind a tagged header, since bringing the game heaps up calls SDL before SDL is ready. The tracking heap's
bookkeeping map also moved to `malloc`, because allocating through the replaced `operator new` re-entered the
memory manager and deadlocked two heaps against each other.

**Boot blockers.** `MAX_CEMENT_LIBRARIES` was 10 while the PC sound path registers 12, so registration tripped
its own assert. macOS gets the 15 that UWP already had. Dialogue registration probes for the numbered `.rcf`
name and falls back to the un-numbered retail one.

**OpenGL on the main thread.** The GL context belongs to the main thread, but the radLoad worker could reach
it three ways, each of which segfaulted mid-level-load. Load-completion callbacks now only run on the thread
that initialised radLoad; texture deletes queue onto a mutex-protected list drained at the top of
`BeginFrame`; and ambient light set from the loader thread is stored and re-applied on the GL thread. A
`PDDIASSERT` in `pglTexture::SetGLState` catches any remaining offender in debug builds.

**Textures.** `tDXTNHandler` was registered only for Windows and UWP, so there was no handler for the DDS
images in the retail p3d files and sprite loading dereferenced a null texture. Registering it was all that was
needed — Apple's GL 2.1 profile does expose `EXT_texture_compression_s3tc`. Separately, uncompressed textures
were uploaded as `GL_SRGB8` / `GL_SRGB8_ALPHA8`, so the GPU decoded them to linear on sampling while nothing
ever re-encoded on output, which made the whole image dark with crushed shadows. The renderer is not gamma
correct by design, so those go back to `GL_RGB8` / `GL_RGBA8`.

## Other Platforms

Windows and Xbox are upstream's territory; this fork does not change those paths and does not test them.
The instructions below are kept from upstream for reference.

### Pre-built binaries

**Desktop.** Download the latest build from upstream's
[Releases page](https://github.com/3UR/Simpsons-Hit-Run/releases/latest), extract it, and run `SRR2.exe`.

**Xbox Series S|X.** Download an `SRR2_UWP_X.X.X.X_x64_XXX.appx` from the same
[Releases page](https://github.com/3UR/Simpsons-Hit-Run/releases/latest), open the Xbox dev mode portal
(https://xbox:11443/ or whatever local IP and port the dev mode dashboard shows), press "Add", and drag the
AppX in.

> [!WARNING]
> Xbox One is not supported and will crash. Xbox Series S|X is fine.

### Building on Windows

You will need Visual Studio 2026 with the `Desktop development with C++` and
`Universal Windows Platform development` workloads, and the `Vcpkg package manager` component.

1. `git clone --recurse-submodules https://github.com/3UR/Simpsons-Hit-Run`
2. Open `SRR2.sln`.
3. `Tools -> Command Line -> Developer Command Prompt`.
4. `cd tools/vcpkg` then `./bootstrap-vcpkg.bat`.
5. `./vcpkg integrate install`.

You can then build any project in the solution. When building for UWP, remember to change the configuration
(for example `ReleaseWindows` becomes `ReleaseUwp`).

## Disclaimer

This repository contains modified source code and references to original game assets for preservation, and enhancement purposes only.

- This project is **not affiliated with, endorsed by, or sponsored by** the original publishers or intellectual property owners.
- No ownership of the original intellectual property is claimed.
- This project is **not monetized** in any way (no sales, ads, sponsorships, or donations).
- Commercial redistribution of this project or its builds is not permitted.

_The Simpsons: Hit & Run_ and all related characters, assets, audio, trademarks, and branding remain the property of their respective rights holders.

## Contributing

Contributions are welcome. Please [create a fork](https://github.com/ddoodm/The-Simpsons-Hit-and-Run-MacOS/fork) and then [open a pull request](https://github.com/ddoodm/The-Simpsons-Hit-and-Run-MacOS/pulls).

Bugs in code inherited from upstream are usually better reported [there](https://github.com/3UR/Simpsons-Hit-Run/issues) unless they are macOS-specific.

## Issues

If you encounter issues, please [create an issue](https://github.com/ddoodm/The-Simpsons-Hit-and-Run-MacOS/issues/new).

## Commit History

This repository starts from upstream's squashed history. Upstream's earlier commit history is archived in [this branch](https://github.com/3UR/Simpsons-Hit-Run/tree/commit-history-archive).

## Media

### MacOS

<img width="912" height="744" alt="Screenshot 2026-09-06 at 7 12 07 pm" src="https://github.com/user-attachments/assets/93800223-a9a5-409e-9e5f-beeff081eac1" />

<img width="912" height="744" alt="Screenshot 2026-09-06 at 7 12 41 pm" src="https://github.com/user-attachments/assets/e48a9dc0-4fae-487c-9347-e404139908e6" />


### Windows
![Screenshot 2025-02-10 092453](https://github.com/user-attachments/assets/7b5c9c6a-259d-4e5d-bd07-e429bd2f54bb)

### Xbox
[_Watch HD on YouTube_](https://www.youtube.com/watch?v=qxqnziUVz9c)

https://github.com/user-attachments/assets/9793dccf-5dd6-4bbf-beb6-a6db33521a0b

[_Watch HD on YouTube_](https://www.youtube.com/watch?v=l_Ii-4Wygn8)

https://github.com/user-attachments/assets/ccfdb377-10ed-418b-a81b-932aad9938e1
