<img src="packaging/resources/icon.png" width="200" height="200">

# Spectral Shift

Spectral Shift is a pitch-shifting plugin built with [JUCE](https://juce.com/) and Signalsmith’s
[Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch) library.

The project was developed as part of the [MyWorld](https://www.myworld-creates.com/) Fellowship in Residence with
[Wounded Buffalo Studios](https://woundedbuffalo.co.uk/).

_More info about the fellowship can be found [here](https://github.com/trencrumb/MyWorldFellowship)._

It’s loosely inspired by [Minimal Audio’s Formant Shift](https://www.minimal.audio/products/formant?srsltid=AfmBOoohvoIIFyv9GE80mIuqKwkI86pF-FlCWWH56WRS87ijlaBsKbu1), but takes a different approach by combining pitch and formant shifting into a 2D control space.

<img src="resources/gui.png" width="400">

## Installation

### macOS

Tested on macOS 15.6.1 (Apple Silicon).

1. Download the latest release from the [Releases](https://github.com/trencrumb/SpectralShift/releases) page.
2. Open the `.pkg` installer and follow the prompts.
3. The build is a universal binary and supports both Intel and Apple Silicon Macs.

### Linux

Tested on Ubuntu 24.04 (x86_64).

1. Download the latest release from the [Releases](https://github.com/trencrumb/SpectralShift/releases) page.
2. Extract the archive.
3. For a quick install, run:

   ```bash
   ./install.sh -y
   ```
4. To uninstall:

   ```bash
   ./uninstall.sh
   ```

### Windows

Tested on Windows 25H2 (x86_64).

1. Download the latest release from the [Releases](https://github.com/trencrumb/SpectralShift/releases) page.
2. Run the `.exe` installer and follow the instructions.
3. The installer is not code signed, so Windows may show a warning about an unknown publisher. This is expected.

## Building from Source

### Prerequisites

#### All platforms

* CMake 3.24 or newer
* Git
* C++20-compatible compiler

#### macOS

* Xcode (latest stable recommended)
* Ninja (optional but much faster build times):

  ```bash
  brew install ninja
  ```

#### Linux (Ubuntu / Debian)

* GCC or Clang with C++20 support
* Ninja (optional):

  ```bash
  sudo apt install ninja-build
  ```
* JUCE dependencies:

  ```bash
  sudo apt-get update && sudo apt install \
    libasound2-dev \
    libx11-dev \
    libxinerama-dev \
    libxext-dev \
    libfreetype6-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev
  ```

#### Windows

* Visual Studio 2019 or newer (Desktop development with C++)
* CMake
* Ninja (optional, via Chocolatey):

  ```powershell
  choco install ninja
  ```

### Build Instructions

#### macOS / Linux

```bash
git clone https://github.com/trencrumb/SpectralShift.git
cd SpectralShift
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release
```

If `COPY_PLUGIN_AFTER_BUILD` is enabled, plugins are copied to 

#### Windows

```bash
git clone https://github.com/trencrumb/SpectralShift.git
cd SpectralShift
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release
```

Built plugins can be found in:

```
build/SpectralShift_artefacts/Release/
```

### Build Options

* Enable PFFFT on Linux:

  ```bash
  cmake -B build -DUSE_PFFFT=ON
  ```
* Limit build formats:
  Edit the `PLUGIN_FORMATS` line in `CMakeLists.txt`

### Automatic Dependencies

Dependencies are fetched automatically via CPM:

* JUCE
* signalsmith-stretch
* clap-juce-extensions
* PFFFT (optional on Linux mainly)
* Perfetto (optional for debugging)

## Troubleshooting

**macOS: “Apple Clang 16.0.x detected” warning**
Dont worry. `-ffast-math` is disabled for this compiler due to a known SIMD issue mentioned in signalsmith's repo listed below.

**Linux: missing JUCE dependencies**
Ensure all prerequisite packages are installed, or install the equivalent packages for your distribution, I've only tested with Ubuntu and Debian.

**Windows: Visual Studio build failures**
Make sure the *Desktop development with C++* workload is installed from Visual Studio Installer.

## References
* [JUCE](https://juce.com/)
* [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch)
* [Pamplejuce](https://github.com/sudara/pamplejuce)
* [macOS Code Signing](https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/)
* [Perfetto-JUCE](https://melatonin.dev/blog/using-perfetto-with-juce-for-dsp-and-ui-performance-tuning/)
* [2D Slider](https://github.com/Thrifleganger/xy-pad-demo)
* [TiltEQ](https://github.com/jcurtis4207/Juce-Plugins)

## License

Released under the [MIT License](LICENSE.txt).
Feel free to get in touch if you have questions or run into issues.
