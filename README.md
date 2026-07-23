[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL--2.0--or--later-blue.svg)](LICENSE)
[![CMake](https://img.shields.io/badge/CMake-3.23%2B-064F8C.svg)](https://cmake.org/)
[![Documentation](https://img.shields.io/badge/docs-online-blue.svg)](https://piximoon.github.io/miniexiv/)
[![GitHub release](https://img.shields.io/github/v/release/PixiMoon/miniexiv)](https://github.com/PixiMoon/miniexiv/releases)

# MiniExiv

MiniExiv is a lightweight C wrapper around the [Exiv2](https://github.com/Exiv2/exiv2) metadata library.

It provides a simple C API for reading and modifying image metadata while using Exiv2 internally.

> MiniExiv aims to provide a stable C API and ABI on top of the Exiv2 C++ library, making Exiv2 easier to use from C and other programming languages.

- [✨ Features](#-features)
- [🛠 Requirements](#-requirements)
- [📦 Installing Dependencies](#-installing-dependencies)
- [🚀 Building](#-building)
- [🧪 Running Tests](#-running-tests)
- [💾 Installation](#-installation)
- [🔗 Using MiniExiv](#-using-miniexiv)
- [⚡ Quick Start](#-quick-start)
- [🧠 Memory Management](#-memory-management)
- [🔒 Thread Safety](#-thread-safety)
- [📖 Documentation](#-documentation)
- [📦 Packaging](#-packaging)
- [📄 License](#-license)

## ✨ Features

MiniExiv exposes the core metadata functionality of Exiv2 through a lightweight C interface:

- Read and write EXIF metadata
- Read and write IPTC metadata
- Read and write XMP metadata
- Remove individual metadata entries
- Clear EXIF, IPTC, and XMP metadata
- Read and remove ICC profiles
- Read and modify image comments
- Open images from files
- Open images from memory buffers
- Iterate over EXIF, IPTC, and XMP entries
- Export modified image data to memory
- Save metadata changes back to an image

This makes it possible to use Exiv2 without exposing its C++ API directly to applications or language bindings.

## 🛠 Requirements

- C99-compatible compiler for applications using the public API
- C++17-compatible compiler for building MiniExiv
- CMake 3.23 or newer
- Exiv2 development package
- `pkg-config` where required by the platform
- Ninja, Make, Xcode, Visual Studio, or another CMake-supported build tool

Tested on:

- Debian 12
- Ubuntu
- Fedora
- Arch Linux
- Alpine Linux
- macOS with Homebrew

## 📦 Installing Dependencies

MiniExiv requires a C/C++ toolchain, CMake, Exiv2 development files, and a supported build tool.

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    ninja-build \
    build-essential \
    pkg-config \
    libexiv2-dev
```

### macOS

Install the Apple command-line tools:

```bash
xcode-select --install
```

Install the remaining dependencies with Homebrew:

```bash
brew install cmake ninja pkg-config exiv2
```

### Fedora

```bash
sudo dnf install -y \
    git \
    cmake \
    ninja-build \
    gcc-c++ \
    pkgconf-pkg-config \
    exiv2-devel
```

### Arch Linux

```bash
sudo pacman -S --needed \
    git \
    cmake \
    ninja \
    gcc \
    pkgconf \
    exiv2
```

### Alpine Linux

```bash
sudo apk add \
    git \
    cmake \
    ninja \
    build-base \
    pkgconf \
    exiv2-dev
```

## 🚀 Building

Clone the repository:

```bash
git clone https://github.com/PixiMoon/miniexiv.git
cd miniexiv
```

### Shared release build

```bash
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON

cmake --build build
```

### Static release build

```bash
cmake -S . -B build-static -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF

cmake --build build-static
```

### Debug build

```bash
cmake -S . -B build-debug -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_SHARED_LIBS=ON

cmake --build build-debug
```

### CMake options

| Option | Purpose | Default |
|---|---|---:|
| `CMAKE_INSTALL_PREFIX` | Installation directory | Platform-dependent |
| `CMAKE_BUILD_TYPE` | Build configuration for single-config generators | Not set |
| `BUILD_SHARED_LIBS` | Build a shared library when `ON`, or a static library when `OFF` | `OFF` unless set explicitly |
| `BUILD_TESTING` | Build the test suite | `ON` |
| `BUILD_DOCS` | Build Doxygen documentation | `OFF` |

> MiniExiv does not require Ninja specifically. Remove `-G Ninja` to use the platform's default CMake generator.

## 🧪 Running Tests

Configure a debug build with tests enabled:

```bash
cmake -S . -B build-test -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TESTING=ON
```

Build and run the tests:

```bash
cmake --build build-test
ctest --test-dir build-test --output-on-failure
```

## 💾 Installation

### System-wide installation

```bash
sudo cmake --install build
```

### Installation to a custom prefix

```bash
cmake -S . -B build-local -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local"

cmake --build build-local
cmake --install build-local
```

After installation, MiniExiv can be discovered through:

- CMake `find_package()`
- `pkg-config`

When using a custom prefix, you may need to extend the package search paths:

```bash
export CMAKE_PREFIX_PATH="$HOME/.local:$CMAKE_PREFIX_PATH"
export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

The exact library directory may be `lib`, `lib64`, or another platform-specific path.

## 🔗 Using MiniExiv

### CMake

```cmake
cmake_minimum_required(VERSION 3.23)

project(example LANGUAGES C)

find_package(miniexiv CONFIG REQUIRED)

add_executable(example main.c)

target_link_libraries(example
    PRIVATE
        miniexiv::miniexiv
)
```

Configure and build the application:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

### pkg-config

Compile a C application using the installed package:

```bash
cc main.c -o example \
    $(pkg-config --cflags --libs miniexiv)
```

For a static link request:

```bash
cc main.c -o example \
    $(pkg-config --static --cflags --libs miniexiv)
```

> Fully static executables may also require static versions of Exiv2 and all of its transitive dependencies.

## ⚡ Quick Start

```c
#include <stdio.h>

#include <miniexiv/miniexiv.h>
#include <miniexiv/miniexiv_errors.h>

int main(void)
{
    if (miniexiv_initialize() != MINIEXIV_OK) {
        fprintf(
            stderr,
            "MiniExiv initialization failed: %s\n",
            miniexiv_get_last_error()
        );
        return 1;
    }

    miniexiv_image *image = miniexiv_image_open_file("photo.jpg");

    if (image == NULL) {
        fprintf(
            stderr,
            "Unable to open image: %s\n",
            miniexiv_get_last_error()
        );
        miniexiv_shutdown();
        return 1;
    }

    if (miniexiv_image_set_exif_string(
            image,
            "Exif.Image.Artist",
            "Jane Doe"
        ) != MINIEXIV_OK) {
        fprintf(
            stderr,
            "Unable to update EXIF metadata: %s\n",
            miniexiv_get_last_error()
        );

        miniexiv_image_free(image);
        miniexiv_shutdown();
        return 1;
    }

    if (miniexiv_image_metadata_write(image) != MINIEXIV_OK) {
        fprintf(
            stderr,
            "Unable to write metadata: %s\n",
            miniexiv_get_last_error()
        );

        miniexiv_image_free(image);
        miniexiv_shutdown();
        return 1;
    }

    miniexiv_image_free(image);
    miniexiv_shutdown();

    return 0;
}
```

Metadata setter functions modify the in-memory image object. Call the appropriate write, save, or export function to persist or retrieve the modified image.

## 🧠 Memory Management

MiniExiv uses explicit ownership rules.

| Resource | Release operation |
|---|---|
| `miniexiv_image *` | `miniexiv_image_free()` |
| Exported image buffer | Release it with the MiniExiv buffer-free function documented by the API |
| EXIF iterator | Release it with the corresponding EXIF iterator-free function |
| IPTC iterator | Release it with the corresponding IPTC iterator-free function |
| XMP iterator | Release it with the corresponding XMP iterator-free function |

Always use MiniExiv's matching release function for memory returned by MiniExiv.

### Images opened from memory

`miniexiv_image_open_buf()` initially references the caller-provided input buffer.

The caller must keep that buffer alive until the first successful call to:

```c
miniexiv_image_metadata_write(image);
```

After the first successful metadata write, MiniExiv switches the image to an internally owned copy and the original caller buffer may be released.

If the write fails, assume that the original buffer is still referenced.

### Iterator validity

Metadata iterators may become invalid after metadata is added, removed, cleared, or rewritten.

Create a new iterator after modifying the associated metadata container.

## 🔒 Thread Safety

| Context | Safe? | Notes |
|---|---:|---|
| `miniexiv_initialize()` and `miniexiv_shutdown()` | ✅ | Thread-safe and reference-counted |
| Different image handles used by different threads | ✅ | Each handle must be used independently |
| One image handle used concurrently | ❌ | Requires external synchronization |
| One iterator used concurrently | ❌ | Do not share iterators between threads |
| Last-error storage | ✅ | Stored separately for each thread |

A single `miniexiv_image` handle must not be read or modified concurrently without external locking.

## 📖 Documentation

The full API documentation is available online:

➡️ [MiniExiv API Documentation](https://piximoon.github.io/miniexiv/)

### Build documentation locally

Install Doxygen and configure the project:

```bash
cmake -S . -B build-docs -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_DOCS=ON
```

Generate the documentation:

```bash
cmake --build build-docs --target docs
```

The generated HTML documentation is placed in the Doxygen output directory configured by `docs/Doxyfile.in`.

## 📦 Packaging

MiniExiv uses CPack to generate platform-specific archives or packages.

Configure and build a release version:

```bash
cmake -S . -B build-package -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON

cmake --build build-package
```

Generate the package:

```bash
cpack --config build-package/CPackConfig.cmake
```

Depending on the platform, CPack may generate formats such as:

- `.tar.gz`
- `.deb`
- `.zip`

## 📄 License

MiniExiv is licensed under the **GNU General Public License version 2.0 or later** (`GPL-2.0-or-later`).

See the [LICENSE](LICENSE) file for details.
