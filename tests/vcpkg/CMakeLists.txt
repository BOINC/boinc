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

cmake_minimum_required(VERSION 3.21)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
endif()
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/ports" CACHE STRING "vcpkg overlay ports")

project(vcpkg_test C CXX)

find_package(boinc REQUIRED)

add_executable(vcpkg_test main.cpp)
add_executable(vcpkg_c_test main.c)

target_link_libraries(vcpkg_test PRIVATE unofficial::boinc::boinc unofficial::boinc::boincapi)
target_link_libraries(vcpkg_c_test PRIVATE unofficial::boinc::boinc unofficial::boinc::boincapi)
