cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------
# 1. Automatic Git Version Extraction
# ------------------------------------------------------------------------------
execute_process(
    COMMAND git describe --tags --match "v*" --always
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_RAW_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)


if(GIT_RAW_VERSION MATCHES "^v?([0-9]+\\.[0-9]+\\.[0-9]+)")
    set(MINIEXIV_DETECTED_VERSION "${CMAKE_MATCH_1}")
else()
    
    set(MINIEXIV_DETECTED_VERSION "0.5.0")
endif()

project(miniexiv
    VERSION ${MINIEXIV_DETECTED_VERSION}
    DESCRIPTION "C wrapper for Exiv2 library"
    LANGUAGES C CXX
)

message(STATUS "miniexiv version: ${PROJECT_VERSION} (from git tag: ${GIT_RAW_VERSION})")

include(GNUInstallDirs)
# ------------------------------------------------------------------------------
# Standards
# ------------------------------------------------------------------------------

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

option(BUILD_DOCS "Build documentation" OFF)

if(BUILD_DOCS)

    find_package(Doxygen REQUIRED)

    if(DOXYGEN_FOUND)

        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(
            ${DOXYGEN_IN}
            ${DOXYGEN_OUT}
            @ONLY
        )

        add_custom_target(
            docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )

    endif()

endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)

    pkg_check_modules(EXIV2 QUIET exiv2)

    if(EXIV2_FOUND)

        message(STATUS "Found Exiv2 via pkg-config")

        add_library(exiv2::exiv2 INTERFACE IMPORTED)

        set_target_properties(exiv2::exiv2 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EXIV2_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${EXIV2_LIBRARIES}"
        )

        set(EXIV2_TARGET exiv2::exiv2)

    endif()

endif()
# Try CMake package (newer Exiv2)
# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

option(MINIEXIV_FETCH_DEPENDENCIES 
    "Download missing dependencies automatically"
    YES
)

set(EXIV2_TARGET "")


# ------------------------------------------------------------------------------
# 1. Try installed Exiv2
#    - vcpkg    
#    - system packages
#    - Homebrew
#    - custom install
# ------------------------------------------------------------------------------

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


# ------------------------------------------------------------------------------
# 2. Fallback: download Exiv2
# ------------------------------------------------------------------------------

if(NOT EXIV2_TARGET AND MINIEXIV_FETCH_DEPENDENCIES)

    message(STATUS
        "Exiv2 not found. Fetching dependency..."
    )

    include(FetchContent)


    set(EXIV2_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(EXIV2_BUILD_EXIV2_COMMAND OFF CACHE BOOL "" FORCE)
    set(EXIV2_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(EXIV2_ENABLE_NLS OFF CACHE BOOL "" FORCE)


    FetchContent_Declare(
        exiv2
        GIT_REPOSITORY https://github.com/Exiv2/exiv2.git
        GIT_TAG v0.28.8
    )


    FetchContent_MakeAvailable(exiv2)


    if(TARGET Exiv2::exiv2lib)

        set(EXIV2_TARGET Exiv2::exiv2lib)

    elseif(TARGET Exiv2::exiv2)

        set(EXIV2_TARGET Exiv2::exiv2)

    endif()

endif()


# ------------------------------------------------------------------------------
# 3. Final check
# ------------------------------------------------------------------------------

if(NOT EXIV2_TARGET)

    message(FATAL_ERROR
        "
        Exiv2 was not found.

        Install Exiv2 manually or enable:
        -DMINIEXIV_FETCH_DEPENDENCIES=ON
        "
    )

endif()




message(STATUS "Using Exiv2 target: ${EXIV2_TARGET}")
# ------------------------------------------------------------------------------
# Library
# ------------------------------------------------------------------------------

add_library(miniexiv 
    src/miniexiv.cpp
    src/miniexiv_errors.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)
set_target_properties(miniexiv PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(miniexiv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

# Modern Exiv2 packages export one of these targets
target_link_libraries(miniexiv PRIVATE ${EXIV2_TARGET})

# ------------------------------------------------------------------------------
# Visibility
# ------------------------------------------------------------------------------

set_target_properties(miniexiv PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# ------------------------------------------------------------------------------
# Export header
# ------------------------------------------------------------------------------

include(GenerateExportHeader)

generate_export_header(miniexiv
    BASE_NAME miniexiv
    EXPORT_FILE_NAME
        ${CMAKE_CURRENT_BINARY_DIR}/include/miniexiv/miniexiv_export.h
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    @ONLY
)

# ------------------------------------------------------------------------------
# CMake package configuration
# ------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/miniexivConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)


configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/miniexivConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/miniexivConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv
)
# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/miniexiv.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/miniexiv.pc
    @ONLY
)


install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/miniexiv.pc
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)


install(TARGETS miniexiv
    EXPORT miniexivTargets

    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miniexiv/

    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/miniexiv

    FILES_MATCHING
        PATTERN "*.h"
)


install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/include/miniexiv/miniexiv_export.h

    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/miniexiv
)



# ------------------------------------------------------------------------------
# CMake package export
# ------------------------------------------------------------------------------

export(
    EXPORT miniexivTargets

    FILE
        ${CMAKE_CURRENT_BINARY_DIR}/miniexivTargets.cmake

    NAMESPACE
        miniexiv::
)


install(
    EXPORT miniexivTargets

    FILE
        miniexivTargets.cmake

    NAMESPACE
        miniexiv::

    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv
)



install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/miniexivConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/miniexivConfigVersion.cmake

    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/miniexiv
)



# ------------------------------------------------------------------------------
# Documentation files
# ------------------------------------------------------------------------------

install(
    FILES
        README.md
        LICENSE

    DESTINATION
        share/doc/miniexiv
)



# ------------------------------------------------------------------------------
# Packaging
# ------------------------------------------------------------------------------

set(CPACK_PACKAGE_NAME
    "miniexiv"
)

set(CPACK_PACKAGE_VENDOR
    "PixiMoon"
)


set(CPACK_PACKAGE_VERSION
    ${PROJECT_VERSION}
)


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Lightweight C wrapper around Exiv2 metadata library"
)


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    )
endif()


set(CPACK_SYSTEM_NAME
    "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
)


set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}"
)



if(UNIX AND NOT APPLE)

    set(CPACK_GENERATOR
        "TGZ;DEB"
    )


    set(CPACK_DEBIAN_PACKAGE_MAINTAINER
        "PixiMoon"
    )


    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libexiv2-27"
    )


elseif(APPLE)


    set(CPACK_GENERATOR
        "TGZ"
    )


elseif(WIN32)


    set(CPACK_GENERATOR
        "ZIP"
    )


endif()



include(CPack)



# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

include(CTest)


if(BUILD_TESTING)

    add_subdirectory(tests)

endif()