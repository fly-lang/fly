  [![Build Status](https://github.com/fly-lang/fly/actions/workflows/build.yml/badge.svg)](https://github.com/fly-lang/fly/actions/workflows/build.yml)
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
- [CMake (min version 3.4.3)](https://cmake.org)
- C++ 14

## Usage
This is an example of how to configure and build the Fly source.
1. Checkout Fly:
    
    * ``git clone https://github.com/fly-lang/fly.git``
   
    * Or, on windows, ``git clone --config core.autocrlf=false``
2. Linux (Ubuntu 22.04) building packages:
    
   * Build dependencies: ``sudo apt install build-essential libxml2-dev zlib1g-dev libtinfo-dev``
   * Add stacktrace (debug purpose): ``sudo apt install binutils-dev libdw-dev libdwarf-dev``

3. Configure and build:
   
   * ``cd fly``

   * ``mkdir build``

   * ``cd build``

   * ``cmake ..``
     
   * ``cmake --build . ``
   
4. Launch Fly tests:
   
   * ``ctest``

You can build Fly with your installed [LLVM 11](https://github.com/llvm/llvm-project) (fastest method) or automatically 
compiled from source.

For more information see [CMake](https://llvm.org/docs/CMake.html)

## Additional Info
- For how to contribute see `CONTRIBUTING.md`
- For know Authors see `AUTHORS.md`
- See `LICENSE`

## Contact
BlueSky: [@flylang.org](https://bsky.app/profile/flylang.org)

Email: [dev@flylang.org](mailto:dev@flylang.org)

Website: [flylang.org](https://flylang.org)
 

