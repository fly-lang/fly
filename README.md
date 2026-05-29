  [![Build Linux](https://github.com/fly-lang/fly/actions/workflows/build-linux.yml/badge.svg)](https://github.com/fly-lang/fly/actions/workflows/build-linux.yml)
  [![Build Windows](https://github.com/fly-lang/fly/actions/workflows/build-windows.yml/badge.svg)](https://github.com/fly-lang/fly/actions/workflows/build-windows.yml)
  ![GitHub issues](https://img.shields.io/github/issues-raw/fly-lang/fly?color=blue)
  ![GitHub closed issues](https://img.shields.io/github/issues-closed/fly-lang/fly?color=blue)
  <br />
<p align="center">
  <a href="https://github.com/fly-lang/fly">
    <img src="https://github.com/fly-lang/graphics/blob/main/logo/fly_logo_300.png?raw=true" alt="Logo" width="300" height="300">
  </a>

  <h3 align="center">Fly Programming Language</h3>

  <p align="center">
    Simple - Fast - Powerfull
    <br />
    <br />
    <a href="https://flylang.org">View Website</a>
    ·
    <a href="https://github.com/fly-lang/fly/issues">Report Bug / Request Feature</a>
    ·
    <a href="https://github.com/fly-lang/fly/discussions">Open a Discussion</a>
  </p>
</p>

## About the Project
Fly Project want to build a new programming language: compiled, high-level, general purpose,
with particular attention to simplicity, readability, multi-paradigms with optional Garbage Collector.

### Build with
This project is a fork of Clang so it is based on LLVM:
- [Clang](https://clang.llvm.org/)
- [LLVM](https://llvm.org/)

## Getting Started
Fly is written in C++ and it is a fork of Clang.

### Prerequisites
In order to build this project you need:
- [CMake (min version 3.24.0)](https://cmake.org)
- C++ 17
- [LLVM 20.1.8](https://github.com/llvm/llvm-project)

## Usage

### Linux

1. Clone the repository:
   ```bash
   git clone https://github.com/fly-lang/fly.git
   ```

2. Install build dependencies for your distribution:

   **Debian / Ubuntu**
   ```bash
   sudo apt install build-essential libxml2-dev zlib1g-dev libtinfo-dev
   # stacktrace support (debug purpose)
   sudo apt install binutils-dev libdw-dev libdwarf-dev
   ```

   **Fedora**
   ```bash
   sudo dnf install gcc gcc-c++ make libxml2-devel zlib-devel ncurses-devel
   # stacktrace support (debug purpose)
   sudo dnf install binutils-devel elfutils-devel libdwarf-devel
   ```

   **RHEL / CentOS Stream**
   ```bash
   sudo dnf install gcc gcc-c++ make libxml2-devel zlib-devel ncurses-devel
   # stacktrace support (debug purpose)
   sudo dnf install binutils-devel elfutils-devel libdwarf-devel
   ```

   **openSUSE**
   ```bash
   sudo zypper install gcc gcc-c++ make libxml2-devel zlib-devel ncurses-devel
   # stacktrace support (debug purpose)
   sudo zypper install binutils-devel libdw-devel libdwarf-devel
   ```

   **Arch Linux**
   ```bash
   sudo pacman -S base-devel libxml2 zlib ncurses
   # stacktrace support (debug purpose)
   sudo pacman -S libdwarf elfutils
   ```

3. Configure and build:
   ```bash
   cd fly
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

4. Run tests (must be run from the build directory):
   ```bash
   ctest
   ```

### macOS

1. Clone the repository:
   ```bash
   git clone https://github.com/fly-lang/fly.git
   ```

2. Install Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```

3. Install build dependencies via [Homebrew](https://brew.sh):
   ```bash
   brew install cmake libxml2 zlib
   ```

4. Configure and build:
   ```bash
   cd fly
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

5. Run tests:
   ```bash
   ctest
   ```

### Windows

1. Clone the repository:
   ```bash
   git clone --config core.autocrlf=false https://github.com/fly-lang/fly.git
   ```

2. Install dependencies via vcpkg:
   ```bash
   vcpkg install zstd:x64-windows
   ```

3. Configure and build:
   ```bash
   cd fly
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_INSTALLATION_ROOT%\scripts\buildsystems\vcpkg.cmake"
   cmake --build . --config Release
   ```

4. Run tests:
   ```bash
   ctest -C Release
   ```

## Additional Info
- For how to contribute see `CONTRIBUTING.md`
- For know Authors see `AUTHORS.md`
- See `LICENSE`

## Contact
BlueSky: [@flylang.org](https://bsky.app/profile/flylang.org)

Email: [dev@flylang.org](mailto:dev@flylang.org)

Website: [flylang.org](https://flylang.org)
 

