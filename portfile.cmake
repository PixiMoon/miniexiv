vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO PixiMoon/miniexiv
    REF "v${VERSION}"
    SHA512 @MINIEXIV_SHA512@ 
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME miniexiv
    CONFIG_PATH lib/cmake/miniexiv
)

vcpkg_fixup_pkgconfig()

file(
    REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
)

vcpkg_install_copyright(
    FILE_LIST "${SOURCE_PATH}/LICENSE"
)