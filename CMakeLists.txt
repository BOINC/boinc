cmake_minimum_required(VERSION 2.8)
project(BOINC)

option(build-manager "build GUI manager" ON)
option(build-client "build CLI client" ON)
option(build-server "build server parts" ON)

set(BOINC_MAJOR_VERSION 7)
set(BOINC_MINOR_VERSION 13)
set(BOINC_RELEASE 0)

set(WRAPPER_RELEASE 26016)

set(VBOXWRAPPER_RELEASE 26197)

set(LIBBOINC_VERSION "${BOINC_MAJOR_VERSION}:${BOINC_MINOR_VERSION}:${BOINC_RELEASE}")
set(BOINC_VERSION_STRING "${BOINC_MAJOR_VERSION}.${BOINC_MINOR_VERSION}.${BOINC_RELEASE}")

set(PACKAGE "boinc")
set(PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(PACKAGE_STRING "${CMAKE_PROJECT_NAME} ${BOINC_VERSION_STRING}")
set(PACKAGE_TARNAME "${PACKAGE}")
set(PACKAGE_VERSION "${BOINC_VERSION_STRING}")

find_package(GLUT)
find_package(JPEG)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_BINARY_DIR}/version.h")

include(configuration.cmake)
configure_file(${PROJECT_SOURCE_DIR}/config.h.cmake ${PROJECT_BINARY_DIR}/config.h)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(api)
add_subdirectory(lib)

if(build-manager)
    find_package(wxWidgets COMPONENTS core base REQUIRED)
    if(NOT wxWidgets_FOUND)
        message(FATAL_ERROR "wxWidgets not found but required to build Manager")
    endif()
    include( "${wxWidgets_USE_FILE}" )
    add_subdirectory(clientgui)
endif()

message("done with top file")
