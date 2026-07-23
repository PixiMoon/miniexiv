[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![C99](https://img.shields.io/badge/C-99-green.svg)](https://en.wikipedia.org/wiki/C99)
[![CMake](https://img.shields.io/badge/CMake-3.23%2B-064F8C.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#)
[![Documentation: Doxygen](https://img.shields.io/badge/docs-Doxygen-blue.svg)](#-documentation)
[![Build](https://github.com/piximoon/miniexiv/actions/workflows/build.yml/badge.svg)](https://github.com/piximoon/miniexiv/actions)

[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://piximoon.github.io/miniexiv/)
# MiniExiv

MiniExiv is a lightweight C wrapper around the powerful Exiv2 library. It provides a simple C API for reading and modifying image metadata while leveraging Exiv2 internally.
>The core idea of MiniExiv is to provide a stable C API with ABI compatibility for the Exiv2 library.

- [✨ Features](#-features)
- [🛠 Requirements](#-requirements)
- [📦 Installing Dependencies](#-installing-dependencies)
- [🚀 Building](#-building)
- [💾 Installation](#-installation)
- [⚡ Quick Start](#-quick-start)
- [🔒 Thread Safety](#-thread-safety)
- [📄 License](#-license)

## ✨ Features
The core idea of MiniExiv is to provide a stable C API with ABI compatibility for the Exiv2 library.

MiniExiv lets you work with almost everything that Exiv2 supports, but through a simple C API:

Read and write EXIF, IPTC, and XMP metadata

Remove EXIF, IPTC, and XMP metadata

Read and remove ICC profiles

Read and modify image comments

In practice, this means you can handle nearly all common image metadata tasks that Exiv2 provides, but with a lightweight C interface.

> MiniExiv exposes nearly all of the core functionality of [Exiv2](ca://s?q=Exiv2_library) through a simple C API — making it possible to manage image metadata without diving into C++.


## 🛠 Requirements

- C++17 compatible compiler
- CMake 3.23+
- Ninja
- pkg-config
- [Exiv2](https://github.com/Exiv2/exiv2)

See the full API reference:

https://piximoon.github.io/miniexiv/
## 🛠 Installing Dependencies

MiniExiv requires a C/C++ toolchain, CMake, pkg-config and Exiv2 development files.

The following systems are tested in clean Docker environments:

- Debian 12
- Ubuntu
- Fedora
- Arch Linux
- Alpine Linux

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install -y git cmake ninja-build build-essential pkg-config libexiv2-dev
```
### Mac Os

```bash
xcode-select --install
brew install cmake ninja pkg-config exiv2
```
### Fedora
```bash
sudo dnf install -y git cmake ninja-build gcc-c++ pkgconf-pkg-config exiv2-devel

```
### Arch Linux
```bash
sudo pacman -Sy --needed git cmake ninja gcc pkgconf exiv2
```

### Alpine Linux
```bash
sudo apk add git cmake ninja build-base pkgconf exiv2-dev
```

## 🚀 Building

###  Clone and build
These options are provided by CMake and can be passed during configuration.

| Option | Purpose | Default |
|--------|---------|---------|
| `CMAKE_INSTALL_PREFIX` | Installation directory | `/usr/local` |
| `CMAKE_BUILD_TYPE` | Build type (`Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`) | `Release` |
| `BUILD_SHARED_LIBS` | Build shared (`ON`) or static (`OFF`) library | `ON` |


```bash 
git clone https://github.com/PixiMoon/miniexiv.git
cd miniexiv

cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON

cmake --build build
```
## 💾 Installation

Install system‑wide:

```bash
sudo cmake --install build
```
This makes MiniExiv discoverable via find_package(miniexiv) or pkg-config.
## ⚡ Quick Start

```cpp
#include <iostream>
#include "miniexiv/miniexiv.h"

int main() {
    miniexiv_initialize();

    miniexiv_image* img = miniexiv_image_open_file("photo.jpg");
    if (!img) {
        std::cerr << "Error: " << miniexiv_get_last_error() << "\n";
        miniexiv_shutdown();
        return 1;
    }

    miniexiv_image_set_exif_string(img, "Exif.Image.Artist", "Jane Doe");
    miniexiv_image_metadata_write(img);

    miniexiv_image_free(img);
    miniexiv_shutdown();
    return 0;
}
```

## 🔒 Thread Safety

| Context | Safe? | Notes |
|---------|-------|-------|
| Global lifecycle (`miniexiv_initialize` / `miniexiv_shutdown`) | ✅ Yes | Thread‑safe, reference‑counted |
| Image handles (`miniexiv_image`) | ❌ No | Requires external synchronization (e.g., `std::mutex`) |
## 📖 Documentation

The full API reference is available online:

➡️ [MiniExiv API Documentation](https://piximoon.github.io/miniexiv/)

## Build documentation
```bash
cmake -S . -B build -G Ninja -DBUILD_DOCS=ON
cmake --build build --target docs
```
### Installing Dependencies (Ubuntu / Debian)

Update your package index and install the required system packages:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y \
    git \
    cmake \
    build-essential \
    pkg-config \
    libexiv2-dev
```





## 📄 License

This project is licensed under the **GNU General Public License v2.0 or later (GPL-2.0-or-later)** — the same license used by the underlying [Exiv2](https://github.com/Exiv2/exiv2) library. 

See the [LICENSE](LICENSE) file for details.