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
LIST(APPEND SRCS
    ../../../samples/vboxwrapper/floppyio.cpp
    ../../../samples/vboxwrapper/vbox_common.cpp
    ../../../samples/vboxwrapper/vbox_vboxmanage.cpp
    ../../../samples/vboxwrapper/vboxjob.cpp
    ../../../samples/vboxwrapper/vboxlogging.cpp
    )


add_executable(test_vboxwrapper ${SRCS})

TARGET_LINK_LIBRARIES(test_vboxwrapper "${BOINC_API_LIB}" "${BOINC_LIB}" GTest::gtest GTest::gtest_main)

add_test(NAME test_vboxwrapper COMMAND test_vboxwrapper)
