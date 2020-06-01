include(vcpkg_common_functions)

vcpkg_download_distfile(ARCHIVE
    URLS "https://nanohub.org/app/site/downloads/rappture/rappture-src-20130903.tar.gz"
    FILENAME "rappture-src-20130903.tar.gz"
    SHA512 3b42569d056c5e80762eada3aff23d230d4ba8f6f0078de44d8571a713dde91e31e66fe3c37ceb66e934a1410b338fb481aeb5a29ef56b53da4ad2e8a2a2ae59
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
    PATCHES
    "${CMAKE_CURRENT_LIST_DIR}/fixBuild.patch"
)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/config.h DESTINATION ${SOURCE_PATH}/src/core/)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/unistd.h DESTINATION ${SOURCE_PATH}/src/core/)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(
    INSTALL ${SOURCE_PATH}/license.terms
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/rappture
    RENAME copyright
)
