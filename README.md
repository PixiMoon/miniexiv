[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![C99](https://img.shields.io/badge/C-99-green.svg)](https://en.wikipedia.org/wiki/C99)
[![CMake](https://img.shields.io/badge/CMake-3.23%2B-064F8C.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#)
[![Documentation: Doxygen](https://img.shields.io/badge/docs-Doxygen-blue.svg)](#-documentation)
[![Build](https://github.com/piximoon/miniexiv/actions/workflows/build.yml/badge.svg)](https://github.com/piximoon/miniexiv/actions)

[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://piximoon.github.io/miniexiv/)
# MiniExiv

MiniExiv is a lightweight C wrapper around the powerful [Exiv2](https://github.com/Exiv2/exiv2) library for reading and modifying image metadata. It provides a simple C API while using Exiv2 internally.

## ✨ Features

- Read and write EXIF IPTC XMP metadata
- Remove EXIF/IPTC/XMP metadata
- Read and remove ICC profiles
- Read and modify image comments

## 🛠 Requirements

- C99 compatible compiler
- C++17 compatible compiler
- CMake 3.23+
- [Exiv2](https://github.com/Exiv2/exiv2)

See the full API reference:

https://piximoon.github.io/miniexiv/
## Quick Start

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

MiniExiv handles underlying Exiv2 and Adobe XMP SDK lifecycle safety for you:

- **Global Lifecycle (`miniexiv_initialize` / `miniexiv_shutdown`)**: Thread-safe and reference-counted. Safe to call concurrently from multiple threads or independent sub-modules. The global Exiv2/XMP state is initialized on the first call and destroyed only when the reference count drops to zero.

- **Shared Handle Access**: **Not thread-safe**. Concurrent operations on the **same** `miniexiv_image` handle across multiple threads require external synchronization (e.g., `std::mutex`).
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

## 🚀 Building and Testing


### 1. Clone the Repository
```bash
git clone https://github.com/PixiMoon/miniexiv.git
cd miniexiv
```

### 2. Available Build Presets
You can choose from the following presets depending on your goals:
- `debug-shared` — Debug build, shared library (`.so` / `.dylib` / `.dll`)
- `debug-static` — Debug build, static library (`.a` / `.lib`)
- `release-shared` — Release build (optimized), shared library (`.so` / `.dylib` / `.dll`)
- `release-static` — Release build (optimized), static library (`.a` / `.lib`)


### 3. Configuration and Compilation
Select a preset (for example, `release-shared`) and run the following commands:

```bash
# Configure the project
cmake --preset release-shared

# Build the project
cmake --build --preset release-shared
```

### 4. Running Tests
The project includes test presets that match the build presets. To run the tests after a successful build, execute:

```bash
ctest --preset release-shared
```

## 💾 Installation

You can install MiniExiv system-wide so it can be discovered by `find_package(miniexiv)` or `pkg-config`.

### System-wide Install (Linux / macOS)

Build the project using any release preset and run the `install` target with elevated privileges:

```bash
cmake --install out/release-shared
```

### Generate docs (optional):
```bash
cmake --preset release-shared -DBUILD_DOCS=ON
cmake --build --preset release-shared --target docs
```
## 📄 License

This project is licensed under the **GNU General Public License v2.0 or later (GPL-2.0-or-later)** — the same license used by the underlying [Exiv2](https://github.com/Exiv2/exiv2) library. 

See the [LICENSE](LICENSE) file for details.