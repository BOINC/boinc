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

list(APPEND SRCS
    ../../../client/acct_mgr.cpp
    ../../../client/acct_setup.cpp
    ../../../client/app.cpp
    ../../../client/app_config.cpp
    ../../../client/app_control.cpp
    ../../../client/app_start.cpp
    ../../../client/app_test.cpp
    ../../../client/async_file.cpp
    ../../../client/client_msgs.cpp
    ../../../client/client_state.cpp
    ../../../client/client_types.cpp
    ../../../client/coproc_sched.cpp
    ../../../client/cpu_sched.cpp
    ../../../client/cs_account.cpp
    ../../../client/cs_apps.cpp
    ../../../client/cs_benchmark.cpp
    ../../../client/cs_cmdline.cpp
    ../../../client/cs_files.cpp
    ../../../client/cs_notice.cpp
    ../../../client/cs_platforms.cpp
    ../../../client/cs_prefs.cpp
    ../../../client/cs_proxy.cpp
    ../../../client/cs_scheduler.cpp
    ../../../client/cs_sporadic.cpp
    ../../../client/cs_statefile.cpp
    ../../../client/cs_trickle.cpp
    ../../../client/current_version.cpp
    ../../../client/dhrystone.cpp
    ../../../client/dhrystone2.cpp
    ../../../client/file_names.cpp
    ../../../client/file_xfer.cpp
    ../../../client/gpu_amd.cpp
    ../../../client/gpu_detect.cpp
    ../../../client/gpu_nvidia.cpp
    ../../../client/gpu_opencl.cpp
    ../../../client/gui_http.cpp
    ../../../client/gui_rpc_server.cpp
    ../../../client/gui_rpc_server_ops.cpp
    ../../../client/hostinfo_network.cpp
    ../../../client/http_curl.cpp
    ../../../client/log_flags.cpp
    ../../../client/mac_address.cpp
    ../../../client/net_stats.cpp
    ../../../client/pers_file_xfer.cpp
    ../../../client/project.cpp
    ../../../client/project_list.cpp
    ../../../client/result.cpp
    ../../../client/rr_sim.cpp
    ../../../client/sandbox.cpp
    ../../../client/scheduler_op.cpp
    ../../../client/thread.cpp
    ../../../client/time_stats.cpp
    ../../../client/whetstone.cpp
    ../../../client/work_fetch.cpp
)

set (SRCS_UNIX
    ../../../client/hostinfo_linux.cpp
    ../../../client/hostinfo_unix.cpp
)

set (SRCS_WINDOWS
    ../../../client/hostinfo_win.cpp
)

if (WIN32)
    list(APPEND SRCS ${SRCS_WINDOWS})
else ()
    list(APPEND SRCS ${SRCS_UNIX})
endif ()

add_executable(test_client
    ${SRCS}
)

set (X11_LIBS "")
if (UNIX AND NOT APPLE)
    list(APPEND X11_LIBS
        X11::X11
        X11::Xss
    )
endif ()

TARGET_LINK_LIBRARIES(test_client "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" "${BOINC_ZIP_LIB}" pthread GTest::gtest GTest::gtest_main CURL::libcurl ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto ${X11_LIBS})

add_test(NAME test_client COMMAND test_client)
