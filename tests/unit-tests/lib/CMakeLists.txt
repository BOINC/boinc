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

file(GLOB SRCS *.cpp)

add_executable(test_lib ${SRCS})

if (LINUX)
    set(LIBZIP_LIBRARIES PkgConfig::LIBZIP)
else()
    set(LIBZIP_LIBRARIES libzip::zip)
endif()

TARGET_LINK_LIBRARIES(test_lib "${SCHED_LIB}" "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" "${BOINC_ZIP_LIB}" "${LIBZIP_LIBRARIES}" pthread GTest::gtest GTest::gtest_main)

add_test(NAME test_lib COMMAND test_lib)
