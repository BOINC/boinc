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

include(${CMAKE_CURRENT_LIST_DIR}/../../vcpkg_root_find.cmake)
include(${VCPKG_ROOT}/triplets/community/armv6-android.cmake)

set(VCPKG_CMAKE_SYSTEM_VERSION android-16)

set(VCPKG_CXX_FLAGS -DNO_RECVMMSG)
set(VCPKG_C_FLAGS -DNO_RECVMMSG)
