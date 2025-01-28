# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2023 University of California
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

if(DEFINED VCPKG_ROOT)
  message(STATUS "vcpkg root directory already defined")
elseif(DEFINED VCPKG_ROOT_DIR)
  set(VCPKG_ROOT ${VCPKG_ROOT_DIR} CACHE PATH "vcpkg root")
elseif(CMAKE_TOOLCHAIN_FILE)
  get_filename_component(VCPKG_ROOT ${CMAKE_TOOLCHAIN_FILE} DIRECTORY)
  get_filename_component(VCPKG_ROOT ${VCPKG_ROOT} DIRECTORY)
elseif(DEFINED ENV{VCPKG_ROOT})
  set(VCPKG_ROOT $ENV{VCPKG_ROOT} CACHE PATH "vcpkg root")
elseif(DEFINED ENV{USERPROFILE})
  set(VCPKG_ROOT "$ENV{USERPROFILE}/vcpkg" CACHE PATH "vcpkg root")
elseif(DEFINED ENV{HOME})
  set(VCPKG_ROOT "$ENV{HOME}/vcpkg" CACHE PATH "vcpkg root")
else()
  message(FATAL_ERROR "Could not find vcpkg root directory")
endif()

message(STATUS "vcpkg root directory: ${VCPKG_ROOT}")
