if(NOT X_VCPKG_FORCE_VCPKG_X_LIBRARIES AND NOT VCPKG_TARGET_IS_WINDOWS)
    message(STATUS "Utils and libraries provided by '${PORT}' should be provided by your system! Install the required packages or force vcpkg libraries by setting X_VCPKG_FORCE_VCPKG_X_LIBRARIES in the triplet!")
    set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
else()

vcpkg_from_gitlab(
    GITLAB_URL "https://gitlab.freedesktop.org/xorg"
    OUT_SOURCE_PATH SOURCE_PATH
    REPO "lib/libxtrans"
    REF "xtrans-${VERSION}"
    SHA512 c7037cb6d2fb641486a43c9203949edec2038735ba758f8556add63598dbb3205166a2ec272700639884b1952642c171806e3dab566722cadd4c71ca98c0a1bf
    HEAD_REF master
)

# we need both *.h and *.c files to be in the destination include folder
# this is by the library design that is actually not a library but a shared code
file(REMOVE_RECURSE ${SOURCE_PATH}/doc)
file(INSTALL ${SOURCE_PATH}/ DESTINATION ${CURRENT_PACKAGES_DIR}/include/X11/Xtrans FILES_MATCHING PATTERN "*.h" PATTERN "*.c")

# Handle copyright
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
endif()
