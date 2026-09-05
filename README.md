# The Simpsons: Hit & Run — macOS

A macOS port of _The Simpsons: Hit & Run_, using CMake in place of the original Visual Studio solution.

## Upstream & Credits

This is a downstream project based on [3UR/Simpsons-Hit-Run](https://github.com/3UR/Simpsons-Hit-Run), which did
the heavy lifting of getting the game onto x64, C++20, vcpkg and OpenGL. All credit for that work goes to
[3UR](https://github.com/3UR) and the contributors to that repository.

This repository has been modified from that source, starting September 2026, to build with CMake and run on macOS.
It is not affiliated with or endorsed by the upstream project, and changes made here are not submitted back to it.
Licensed under GPL-3.0, the same as upstream — see [LICENSE](LICENSE).

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

### Known Bugs

- Memory Corruption (inherited from upstream)
- Assets (Need a good way to include them)

## Commit History

This repository starts from upstream's squashed history. Upstream's earlier commit history is archived in [this branch](https://github.com/3UR/Simpsons-Hit-Run/tree/commit-history-archive).

## Installation

### Quick Installation (Pre-built Binaries)
<!-- TODO: Clean up this section and improve instructions. -->

### Desktop

1. Download the latest build from the [Releases page](https://github.com/3UR/Simpsons-Hit-Run/releases/latest).
2. Extract the contents of the zip file to your desired location.
3. Navigate to the extracted folder and locate `SRR2.exe`.
4. You can now run the game.

### Xbox Series S|X
> [!WARNING]  
> Xbox One is currently not supported and will crash Xbox Series S|X is fine.

1. Navigate to the [Releases page](https://github.com/3UR/Simpsons-Hit-Run/releases/latest).
2. Download a file that looks like `SRR2_UWP_X.X.X.X_x64_XXX.appx`.
3. Now navigate to the Xbox dev mode portal (https://xbox:11443/ or the local IP and port the dev mode dashboard shows you).
4. Then press "Add" and drag and drop the AppX you downloaded.
5. Once done it should run.

### Developer Installation (Building from Source)
When working with the source you will need these installed:

- Visual Studio 2026
- C++ Language Support (In the Visual Studio installer, select the `Desktop development with C++` and `Universal Windows Platform development` options)
- Vcpkg support (When selecting `Desktop development with C++`, make sure `Vcpkg package manager` is selected on the right side)

#### Setup
> [!NOTE]  
> If you have already built Vcpkg projects before with Visual Studio then you can skip everything past step 3.

1. Open command prompt and navigate to a directory where you want to store the source and run `git clone --recurse-submodules https://github.com/3UR/Simpsons-Hit-Run`
2. When done you can open `SRR2.sln` with Visual Studio.
3. Once in Visual Studio press `Tools -> Command Line -> Developer Command Prompt`
4. A Developer Command Prompt will open enter the following `cd tools/vcpkg` and then `./bootstrap-vcpkg.bat`
5. Once that is done run `./vcpkg integrate install`

#### Building
If the setup was successful, you should now be able to build any project in the solution. When building for UWP don't forget to change the config! (example: if it's `ReleaseWindows` make it `ReleaseUwp`).

## Media

### Windows
![Screenshot 2025-02-10 092453](https://github.com/user-attachments/assets/7b5c9c6a-259d-4e5d-bd07-e429bd2f54bb)

### Xbox
[_Watch HD on YouTube_](https://www.youtube.com/watch?v=qxqnziUVz9c)

https://github.com/user-attachments/assets/9793dccf-5dd6-4bbf-beb6-a6db33521a0b

[_Watch HD on YouTube_](https://www.youtube.com/watch?v=l_Ii-4Wygn8)

https://github.com/user-attachments/assets/ccfdb377-10ed-418b-a81b-932aad9938e1
