cmake_minimum_required(VERSION 3.25)

# ------------------------------------------------------------------------------
# Version
# ------------------------------------------------------------------------------

set(MINIEXIV_FALLBACK_VERSION "0.6.0")
set(GIT_RAW_VERSION "")

find_package(Git QUIET)

if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND
            "${GIT_EXECUTABLE}" describe
            --tags
            --match "v[0-9]*.[0-9]*.[0-9]*"
            --always
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE
            GIT_RAW_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(GIT_RAW_VERSION MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)")
    set(MINIEXIV_DETECTED_VERSION "${CMAKE_MATCH_1}")
else()
    set(MINIEXIV_DETECTED_VERSION "${MINIEXIV_FALLBACK_VERSION}")
endif()

project(
    miniexiv
    VERSION "${MINIEXIV_DETECTED_VERSION}"
    DESCRIPTION "Lightweight C wrapper around the Exiv2 metadata library"
    HOMEPAGE_URL "https://github.com/PixiMoon/miniexiv"
    LANGUAGES C CXX
)

message(
    STATUS
    "MiniExiv version: ${PROJECT_VERSION} (Git description: ${GIT_RAW_VERSION})"
)

include(GNUInstallDirs)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(BUILD_SHARED_LIBS "Build MiniExiv as a shared library" ON)
option(BUILD_DOCS "Build Doxygen documentation" OFF)

# ------------------------------------------------------------------------------
# Language standards
# ------------------------------------------------------------------------------

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Documentation
# ------------------------------------------------------------------------------

if(BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_IN
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
    )

    set(DOXYGEN_OUT
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    )

    configure_file(
        "${DOXYGEN_IN}"
        "${DOXYGEN_OUT}"
        @ONLY
    )

    add_custom_target(
        docs
        COMMAND
            "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_OUT}"
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT
            "Generating MiniExiv API documentation"
        VERBATIM
    )
endif()

# ------------------------------------------------------------------------------
# Exiv2 dependency
# ------------------------------------------------------------------------------

set(EXIV2_TARGET "")

# Prefer an installed CMake package.
find_package(Exiv2 QUIET CONFIG)

if(NOT Exiv2_FOUND)
    find_package(exiv2 QUIET CONFIG)
endif()

if(TARGET Exiv2::exiv2lib)
    set(EXIV2_TARGET Exiv2::exiv2lib)
elseif(TARGET Exiv2::exiv2)
    set(EXIV2_TARGET Exiv2::exiv2)
elseif(TARGET exiv2::exiv2)
    set(EXIV2_TARGET exiv2::exiv2)
endif()

# Fall back to pkg-config.
if(NOT EXIV2_TARGET)
    find_package(PkgConfig QUIET)

    if(PkgConfig_FOUND)
        pkg_check_modules(
            EXIV2
            QUIET
            IMPORTED_TARGET
            exiv2
        )

        if(TARGET PkgConfig::EXIV2)
            set(EXIV2_TARGET PkgConfig::EXIV2)
        endif()
    endif()
endif()

if(NOT EXIV2_TARGET)
    message(
        FATAL_ERROR
        "Exiv2 was not found.\n"
        "Install the Exiv2 development package and configure MiniExiv again."
    )
endif()

message(STATUS "Using Exiv2 target: ${EXIV2_TARGET}")

# ------------------------------------------------------------------------------
# Generated sources
# ------------------------------------------------------------------------------

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.cpp"
    @ONLY
)

# ------------------------------------------------------------------------------
# Library
# ------------------------------------------------------------------------------

add_library(
    miniexiv
    src/miniexiv.cpp
    src/miniexiv_errors.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/version.cpp"
)

add_library(miniexiv::miniexiv ALIAS miniexiv)

set_target_properties(
    miniexiv
    PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
)

target_include_directories(
    miniexiv
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(BUILD_SHARED_LIBS)
    target_link_libraries(
        miniexiv
        PRIVATE
            "${EXIV2_TARGET}"
    )
else()
    target_link_libraries(
        miniexiv
        PUBLIC
            "${EXIV2_TARGET}"
    )
endif()

# ------------------------------------------------------------------------------
# Export header
# ------------------------------------------------------------------------------

include(GenerateExportHeader)

generate_export_header(
    miniexiv
    BASE_NAME miniexiv
    EXPORT_FILE_NAME
        "${CMAKE_CURRENT_BINARY_DIR}/include/miniexiv/miniexiv_export.h"
)

# ------------------------------------------------------------------------------
# Package configuration
# ------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/miniexivConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/miniexivConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/miniexivConfig.cmake"
    INSTALL_DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/miniexiv.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/miniexiv.pc"
    @ONLY
)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

install(
    TARGETS miniexiv
    EXPORT miniexivTargets

    RUNTIME
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT Runtime

    LIBRARY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT Runtime
        NAMELINK_COMPONENT Development

    ARCHIVE
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT Development

    INCLUDES
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/include/miniexiv/"
    DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/miniexiv"
    COMPONENT
        Development
    FILES_MATCHING
        PATTERN "*.h"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/include/miniexiv/miniexiv_export.h"
    DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/miniexiv"
    COMPONENT
        Development
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/miniexiv.pc"
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    COMPONENT
        Development
)

install(
    EXPORT miniexivTargets
    FILE miniexivTargets.cmake
    NAMESPACE miniexiv::
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv"
    COMPONENT
        Development
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/miniexivConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/miniexivConfigVersion.cmake"
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv"
    COMPONENT
        Development
)

install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION
        "${CMAKE_INSTALL_DATADIR}/doc/miniexiv"
    COMPONENT
        Documentation
)

# Allow find_package(miniexiv) from the build tree.
export(
    EXPORT miniexivTargets
    FILE
        "${CMAKE_CURRENT_BINARY_DIR}/miniexivTargets.cmake"
    NAMESPACE
        miniexiv::
)

# ------------------------------------------------------------------------------
# Packaging
# ------------------------------------------------------------------------------

set(CPACK_PACKAGE_NAME "miniexiv")
set(CPACK_PACKAGE_VENDOR "PixiMoon")
set(CPACK_PACKAGE_CONTACT "PixiMoon")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")

set(
    CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "${PROJECT_DESCRIPTION}"
)

set(
    CPACK_PACKAGE_DESCRIPTION
    "MiniExiv provides a C API for reading and modifying EXIF, IPTC, XMP, "
    "ICC profiles, and image comments using the Exiv2 metadata library."
)

set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CHECKSUM SHA256)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(
        CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    )
endif()

string(
    TOLOWER
    "${CMAKE_SYSTEM_NAME}"
    MINIEXIV_PACKAGE_SYSTEM
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
    set(MINIEXIV_PACKAGE_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(MINIEXIV_PACKAGE_ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR)
    set(MINIEXIV_PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
else()
    set(MINIEXIV_PACKAGE_ARCH "unknown")
endif()

set(
    CPACK_SYSTEM_NAME
    "${MINIEXIV_PACKAGE_SYSTEM}-${MINIEXIV_PACKAGE_ARCH}"
)

set(
    CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CPACK_SYSTEM_NAME}"
)

# Source archives.
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")

set(
    CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-source"
)

set(
    CPACK_SOURCE_IGNORE_FILES
    "/\\.git/"
    "/\\.github/"
    "/out/"
    "/build/"
    "/build-[^/]*/"
    "/cmake-build-[^/]*/"
    "/\\.idea/"
    "/\\.vscode/"
    "/CMakeUserPresets\\.json$"
    "/\\.DS_Store$"
    "~$"
)

if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")

    # Debian package.
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "PixiMoon")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    if(BUILD_SHARED_LIBS)
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    endif()

    # RPM package.
    set(CPACK_RPM_PACKAGE_LICENSE "GPL-2.0-or-later")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_AUTOREQ ON)
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")

elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

include(CPack)

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
