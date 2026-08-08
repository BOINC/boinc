# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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

LIST(APPEND SRCS "${PROJECT_SOURCE_DIR}/../../client/hostinfo_linux.cpp")

add_executable(test_client ${SRCS})

TARGET_LINK_LIBRARIES(test_client "${SCHED_LIB}" "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" pthread GTest::gtest GTest::gtest_main)

add_test(NAME test_client COMMAND test_client)
