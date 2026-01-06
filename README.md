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
   ./install.sh
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

## GitHub Actions (CI/CD)

GitHub Actions are used to build and package the plugin on all supported platforms.

### Workflows

There are three separate workflows:

* **Build macOS** (`.github/workflows/build-macos.yml`)
  Builds, code signs, and notarizes the macOS installer.
* **Build Linux** (`.github/workflows/build-linux.yml`)
  Builds and packages the Linux binaries.
* **Build Windows** (`.github/workflows/build-windows.yml`)
  Builds the plugin and creates a Windows installer.

### Triggers

Workflows are currently set up for **manual runs only**.
To enable automatic builds, uncomment the relevant lines in each workflow file:

```yaml
on:
  workflow_dispatch:    # Manual trigger
  # push:
  #   branches: [ main ]
  # pull_request:
```

### Running a Workflow Manually

1. Go to the repository on GitHub.
2. Open the **Actions** tab.
3. Select the workflow you want to run.
4. Click **Run workflow**, choose a branch, and start the build.

### Required Secrets (macOS only)

macOS builds require signing and notarization credentials. Add the following secrets under:

**Settings → Secrets and variables → Actions**

| Secret Name              | Description                                                  |
|--------------------------|--------------------------------------------------------------|
| DEV_ID_CERT              | Base64-encoded Developer ID Application certificate (`.p12`) |
| DEV_ID_CERT_PASSWORD     | Password for the `.p12` file                                 |
| DEVELOPER_ID_APPLICATION | Developer ID Application identity name                       |
| DEVELOPER_ID_INSTALLER   | Developer ID Installer identity name                         |
| NOTARIZATION_USERNAME    | Apple ID email                                               |
| NOTARIZATION_PASSWORD    | App-specific password                                        |
| APPLE_TEAM_ID            | Apple Developer Team ID                                      |

#### Exporting certificates as Base64

1. Export both the **Developer ID Application** and **Developer ID Installer** certificates from Keychain Access or Xcode as a single `.p12` file.
2. Convert to Base64:

   ```bash
   base64 -i Certificates.p12 | pbcopy
   ```
3. Paste the result into the `DEV_ID_CERT` secret.

Linux and Windows builds do not require any secrets.

### Build Artifacts

To download build outputs:

1. Open the **Actions** tab.
2. Select a completed workflow run.
3. Scroll to the **Artifacts** section.
4. Download the installer for your platform:

    * macOS: `Spectral Shift-*version*-macOS.pkg`
    * Linux: `Spectral Shift-*version*-Linux.zip`
    * Windows: `Spectral Shift-*version*-Windows.exe`

## Customizing Builds

### Change the version number

Edit `CMakeLists.txt`:

```cmake
project(SpectralShift VERSION 0.0.0)
```

### Modify plugin formats

Edit the `PLUGIN_FORMATS` variable in `CMakeLists.txt`:

```cmake
set(PLUGIN_FORMATS VST3 AU AUv3 LV2 Standalone)
```

## Building from Source (local)

### Prerequisites

#### All platforms

* CMake 3.24 or newer
* Git (with submodules)
* C++20-compatible compiler

#### macOS

* Xcode (latest stable recommended)
* Ninja (optional):

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

If `COPY_PLUGIN_AFTER_BUILD` is enabled, plugins are copied to:

* macOS: `~/Library/Audio/Plug-Ins/`
* Linux: `~/.vst3/`, `~/.clap/`, `~/.lv2/`, `~/.local/bin/`

#### Windows

Using Ninja:

```bash
git clone https://github.com/trencrumb/SpectralShift.git
cd SpectralShift
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release
```

Using Visual Studio:

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

Built plugins can be found in:

```
build/SpectralShift_artefacts/Release/
```

### Build Options

* Enable Perfetto profiling:

  ```bash
  cmake -B build -DPERFETTO=ON
  ```
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
* PFFFT (optional)
* Perfetto (optional)

## Troubleshooting

**macOS: “Apple Clang 16.0.x detected” warning**
Dont worry. `-ffast-math` is disabled for this compiler due to a known SIMD issue.

**Linux: missing JUCE dependencies**
Ensure all prerequisite packages are installed, or install the equivalent packages for your distribution, I've only tested with Ubuntu and Debian.

**Windows: Visual Studio build failures**
Make sure the *Desktop development with C++* workload is installed from Visual Studio Installer. If issues persist, try building with Ninja.

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
