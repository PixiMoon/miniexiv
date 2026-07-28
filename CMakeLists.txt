cmake_minimum_required(VERSION 3.25)

# ------------------------------------------------------------------------------
# Version
# ------------------------------------------------------------------------------

set(MINIEXIV_FALLBACK_VERSION "0.6.11")
set(GIT_RAW_VERSION "")
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

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

option(BUILD_SHARED_LIBS "Build MiniExiv as a shared library" OFF)
option(BUILD_DOCS "Build Doxygen documentation" OFF)
option(
        MINIEXIV_BUILD_DEPENDENCIES
        "Fetch and build Exiv2 and its dependencies from source"
        ON
)

set(MINIEXIV_REQUESTED_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")

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

    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

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

if(MINIEXIV_BUILD_DEPENDENCIES)
    message(
        STATUS
        "Fetching static zlib, Expat, and Exiv2 dependencies..."
    )

    include(FetchContent)
        set(MINIEXIV_SAVED_SKIP_INSTALL_RULES "${CMAKE_SKIP_INSTALL_RULES}")
set(CMAKE_SKIP_INSTALL_RULES ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)

    # Сохраняем тип библиотеки MiniExiv.
    set(MINIEXIV_REQUESTED_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")

    # Все внутренние зависимости будут статическими.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    # ==========================================================================
    # zlib
    # ==========================================================================

    set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        ZLIB
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.1
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
    )

    # Сначала полностью конфигурируем zlib.
    FetchContent_MakeAvailable(ZLIB)

    if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    elseif(TARGET zlib AND NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB ALIAS zlib)
    endif()

    if(NOT TARGET ZLIB::ZLIB)
        message(FATAL_ERROR "Bundled zlib did not provide a usable target")
    endif()

    # ==========================================================================
    # Expat
    # ==========================================================================

    set(EXPAT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
    set(EXPAT_INSTALL OFF CACHE BOOL "" FORCE)
    set(EXPAT_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXPAT_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        EXPAT
        GIT_REPOSITORY https://github.com/libexpat/libexpat.git
        GIT_TAG R_2_6_0
        SOURCE_SUBDIR expat
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
    )

    # Затем полностью конфигурируем Expat.
    FetchContent_MakeAvailable(EXPAT)

    if(TARGET expat AND NOT TARGET EXPAT::EXPAT)
        add_library(EXPAT::EXPAT ALIAS expat)
    elseif(TARGET expat_static AND NOT TARGET EXPAT::EXPAT)
        add_library(EXPAT::EXPAT ALIAS expat_static)
    elseif(TARGET expat::expat AND NOT TARGET EXPAT::EXPAT)
        add_library(EXPAT::EXPAT ALIAS expat::expat)
    endif()

    if(NOT TARGET EXPAT::EXPAT)
        message(FATAL_ERROR "Bundled Expat did not provide a usable target")
    endif()

    # ==========================================================================
    # Exiv2
    # ==========================================================================

    set(EXIV2_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    set(EXIV2_ENABLE_XMP ON CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_PNG ON CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_BMFF ON CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_BROTLI OFF CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_INIH OFF CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_LENSDATA ON CACHE BOOL "" FORCE)

    set(EXIV2_ENABLE_CURL OFF CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_WEBREADY OFF CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_NLS OFF CACHE BOOL "" FORCE)

    set(EXIV2_BUILD_EXIV2_COMMAND OFF CACHE BOOL "" FORCE)
    set(EXIV2_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(EXIV2_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(EXIV2_BUILD_DOC OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
    exiv2
    GIT_REPOSITORY https://github.com/Exiv2/exiv2.git
    GIT_TAG v0.28.8
    EXCLUDE_FROM_ALL
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
                  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/Exiv2RootCMakeLists.txt"
                  "<SOURCE_DIR>/CMakeLists.txt"
)

    # Exiv2 конфигурируется только после zlib и Expat.
    FetchContent_MakeAvailable(exiv2)
        set(
    CMAKE_SKIP_INSTALL_RULES
    "${MINIEXIV_SAVED_SKIP_INSTALL_RULES}"
)
    if(TARGET exiv2lib)
        set(EXIV2_TARGET exiv2lib)
    elseif(TARGET Exiv2::exiv2lib)
        set(EXIV2_TARGET Exiv2::exiv2lib)
    else()
        message(
            FATAL_ERROR
            "Bundled Exiv2 did not provide the exiv2lib target"
        )
    endif()

    # Восстанавливаем выбранный пользователем тип MiniExiv.
    set(
        BUILD_SHARED_LIBS
        "${MINIEXIV_REQUESTED_BUILD_SHARED_LIBS}"
        CACHE BOOL
        "Build MiniExiv as a shared library"
        FORCE
    )
else()
    message(STATUS "Using externally installed Exiv2")

    find_package(Exiv2 REQUIRED)

    if(TARGET Exiv2::exiv2lib)
        set(EXIV2_TARGET Exiv2::exiv2lib)
    elseif(TARGET exiv2lib)
        set(EXIV2_TARGET exiv2lib)
    else()
        message(FATAL_ERROR "Exiv2 target is unavailable")
    endif()
endif()
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

if(MINIEXIV_BUILD_DEPENDENCIES)
    # Exiv2 0.28.8 generates exiv2lib_export.h in the top-level build folder.
    target_include_directories(
            miniexiv
            PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()

target_link_libraries(
        miniexiv
        PRIVATE
        "${EXIV2_TARGET}"
)

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
# Installation (Унифицировано для обоих режимов)
# ------------------------------------------------------------------------------

install(
        TARGETS miniexiv
        EXPORT miniexivTargets

        RUNTIME
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT MiniExiv

        LIBRARY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT MiniExiv
        NAMELINK_COMPONENT MiniExiv

        ARCHIVE
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT MiniExiv

        INCLUDES
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
        DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/include/miniexiv/"
        DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/miniexiv"
        COMPONENT
        MiniExiv
        FILES_MATCHING
        PATTERN "*.h"
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/include/miniexiv/miniexiv_export.h"
        DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/miniexiv"
        COMPONENT
        MiniExiv
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/miniexiv.pc"
        DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
        COMPONENT
        MiniExiv
)

install(
        EXPORT miniexivTargets
        FILE miniexivTargets.cmake
        NAMESPACE miniexiv::
        DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv"
        COMPONENT
        MiniExiv
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/miniexivConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/miniexivConfigVersion.cmake"
        DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv"
        COMPONENT
        MiniExiv
)

install(
        FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION
        "${CMAKE_INSTALL_DATADIR}/doc/miniexiv"
        COMPONENT
        MiniExiv
)

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
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")

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

set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")

set(
        CPACK_SOURCE_PACKAGE_FILE_NAME
        "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-source"
)

set(
        CPACK_SOURCE_IGNORE_FILES
        "/[.]git/"
        "/[.]github/"
        "/out/"
        "/build/"
        "/build-[^/]*/"
        "/cmake-build-[^/]*/"
        "/[.]idea/"
        "/[.]vscode/"
        "/CMakeUserPresets[.]json$"
        "/[.]DS_Store$"
        "~$"
)

if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")

    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "PixiMoon")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    if(BUILD_SHARED_LIBS)
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    endif()

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