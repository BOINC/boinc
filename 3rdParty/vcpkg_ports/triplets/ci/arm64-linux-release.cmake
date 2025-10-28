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

include(${CMAKE_CURRENT_LIST_DIR}/../../vcpkg_root_find.cmake)
include(${VCPKG_ROOT}/triplets/community/arm64-linux-release.cmake)

set(X_VCPKG_FORCE_VCPKG_X_LIBRARIES ON)

if(PORT STREQUAL "dbus")
    if(NOT $ENV{TMPDIR} STREQUAL "")
        set(DBUS_SESSION_SOCKET_DIR $ENV{TMPDIR})
    elseif(NOT $ENV{TEMP} STREQUAL "")
        set(DBUS_SESSION_SOCKET_DIR $ENV{TEMP})
    elseif(NOT $ENV{TMP} STREQUAL "")
        set(DBUS_SESSION_SOCKET_DIR $ENV{TMP})
    else()
        set(DBUS_SESSION_SOCKET_DIR /tmp)
    endif()
    LIST(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DDBUS_SESSION_SOCKET_DIR=${DBUS_SESSION_SOCKET_DIR}")
endif()
