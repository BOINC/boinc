# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.21)

project(BOINC_unit_testing)
enable_testing()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror --coverage")

if (APPLE)
    find_package(GTest REQUIRED PATHS ${PROJECT_SOURCE_DIR}/../../3rdParty/osx/vcpkg/installed/arm64-osx/share NO_DEFAULT_PATH)
    find_package(libzip REQUIRED PATHS ${PROJECT_SOURCE_DIR}/../../3rdParty/osx/vcpkg/installed/arm64-osx/share NO_DEFAULT_PATH)
else()
    find_package(Threads REQUIRED)
    find_package(GTest REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
endif()

# There is no easy way to interface with the autotools driven build system in boinc
# so the paths below are hardcoded and are mainly suited for building on Travis CI
# TODO: make paths configurable and generate them from boinc make or switch boinc to cmake

include_directories("${PROJECT_SOURCE_DIR}/../../lib")
include_directories("${PROJECT_SOURCE_DIR}/../../zip")
if (APPLE)
    include_directories(BEFORE "${PROJECT_SOURCE_DIR}/../../3rdParty/osx/vcpkg/installed/arm64-osx/include")
else()
    include_directories(BEFORE "${PROJECT_SOURCE_DIR}/../../3rdParty/buildCache/linux/include")
    include_directories("${PROJECT_SOURCE_DIR}/../..")
    include_directories("${PROJECT_SOURCE_DIR}/../../sched")
    include_directories("${PROJECT_SOURCE_DIR}/../../db")
    include_directories("${PROJECT_SOURCE_DIR}/../../tools")
    include_directories("/usr/include/mariadb")
    include_directories("/usr/include/mariadb/mysql")
    include_directories("/usr/include/mysql")
endif()

if (APPLE)
    find_library(BOINC_LIB libboinc.a PATHS ${PROJECT_SOURCE_DIR}/../../build-arm64-osx NO_DEFAULT_PATH)
    find_library(BOINC_ZIP_LIB libboinc_zip.a PATHS ${PROJECT_SOURCE_DIR}/../../build-arm64-osx NO_DEFAULT_PATH)
else()
    find_library(SCHED_LIB libsched.a PATHS ${PROJECT_SOURCE_DIR}/../../sched NO_DEFAULT_PATH)
    find_library(BOINC_CRYPT_LIB libboinc_crypt.a PATHS ${PROJECT_SOURCE_DIR}/../../lib NO_DEFAULT_PATH)
    find_library(BOINC_LIB libboinc.a PATHS ${PROJECT_SOURCE_DIR}/../../lib NO_DEFAULT_PATH)
    find_library(BOINC_ZIP_LIB libboinc_zip.a PATHS ${PROJECT_SOURCE_DIR}/../../zip NO_DEFAULT_PATH)
    find_library(MYSQL_LIB NAMES mariadb mysqlclient)
    message(STATUS "mysql_lib: ${MYSQL_LIB}")
endif()

add_subdirectory(lib)
if (NOT APPLE)
    add_subdirectory(sched)
endif()
