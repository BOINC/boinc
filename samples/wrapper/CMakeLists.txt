# This file is part of BOINC.
# http:#boinc.berkeley.edu
# Copyright (C) 2014 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http:#www.gnu.org/licenses/>.
#
# Contributor: Anna Vyalkova (cyber@sysrq.in)

cmake_minimum_required(VERSION 3.12)

project(boinc-wrapper
    VERSION 26019
    DESCRIPTION "Wrapper to use non-BOINC apps with BOINC"
    LANGUAGES CXX C
)

include(CheckIncludeFile)
include(CheckSymbolExists)
include(FindThreads)

# Compiler configuration

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
elseif(CMAKE_BUILD_TYPE EQUAL "Debug")
    set(DEBUG 1)
endif()

if(APPLE)
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
endif()

if(UNIX)
    add_compile_options(-fPIE -fPIC)
endif()

if(STATIC_LINKING)
    add_link_options(-static-libstdc++)
endif()

add_compile_options(-Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings)

# General options

option(CHECK_EXECUTABLES "Check that executables are signed. Disabling it introduces a security vulnerability" ON)
option(STATIC_LINKING "Link with static version of libstdc++" ON)
option(WITH_SYSTEM_LIBBOINC "Prefer system BOINC libraries" OFF)
set(BOINC_DIR "../.." CACHE STRING "BOINC source root")
set(BOINC_API_DIR "${BOINC_DIR}/api")
set(BOINC_LIB_DIR "${BOINC_DIR}/lib")
set(BOINC_ZIP_DIR "${BOINC_DIR}/zip")

# Dependencies

find_package(PkgConfig)
if(PKG_CONFIG_FOUND AND WITH_SYSTEM_LIBBOINC)
    pkg_check_modules(BOINC IMPORTED_TARGET libboinc_zip libboinc_api libboinc)
endif()

# Config files

check_include_file("sys/resource.h" HAVE_SYS_RESOURCE_H)
check_include_file("sys/resource.h" HAVE_SYS_TYPES_H)
check_include_file("sys/time.h" HAVE_SYS_TIME_H)
check_include_file("sys/wait.h" HAVE_SYS_WAIT_H)

check_symbol_exists(strlcat "string.h" HAVE_STRLCAT)
check_symbol_exists(strlcpy "string.h" HAVE_STRLCPY)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/wrapper_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/wrapper_config.h
)

# Building the application

add_executable(boinc-wrapper
    regerror.c
    regexp.c
    regexp_memory.c
    regexp_report.c
    regsub.c
    wrapper.cpp
)

target_compile_definitions(boinc-wrapper PRIVATE CMAKE)

if(WIN32)  # bundle version info
    enable_language("RC")
    target_sources(boinc-wrapper PRIVATE wrapper_win.rc)
endif()

if(NOT HAVE_STRLCAT)
    target_sources(boinc-wrapper PRIVATE strlcat.c)
endif()
if(NOT HAVE_STRLCPY)
    target_sources(boinc-wrapper PRIVATE strlcpy.c)
endif()

target_link_libraries(boinc-wrapper PRIVATE Threads::Threads)

if(NOT ANDROID AND WITH_SYSTEM_LIBBOINC AND BOINC_FOUND)
    target_link_libraries(boinc-wrapper PRIVATE PkgConfig::BOINC)
else()
    target_include_directories(boinc-wrapper PRIVATE
        ${BOINC_DIR} ${BOINC_ZIP_DIR} ${BOINC_API_DIR} ${BOINC_LIB_DIR}
    )
    target_link_directories(boinc-wrapper PRIVATE
        ${BOINC_ZIP_DIR} ${BOINC_API_DIR} ${BOINC_LIB_DIR}
    )
    target_link_libraries(boinc-wrapper PRIVATE boinc_zip boinc_api boinc)
endif()

# Installing the application

if(UNIX AND NOT ANDROID)
    include(GNUInstallDirs)
    install(TARGETS boinc-wrapper DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(TARGETS boinc-wrapper DESTINATION .)
endif()
