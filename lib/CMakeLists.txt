cmake_minimum_required(VERSION 3.16)
project(boinc-libs)

message(WARNING "This build is under development and may not work as expected")

find_package(OpenSSL REQUIRED)

set(BOINC_DIR ${PROJECT_SOURCE_DIR}/..)

set(LIBBOINC_HEADERS_WIN
    ${BOINC_DIR}/lib/boinc_win.h
    ${BOINC_DIR}/lib/diagnostics_win.h
    ${BOINC_DIR}/lib/idlemon.h
    ${BOINC_DIR}/lib/stackwalker_imports.h
    ${BOINC_DIR}/lib/stackwalker_win.h
)

set(LIBBOINC_HEADERS_UNIX
    ${BOINC_DIR}/lib/synch.h
    ${BOINC_DIR}/lib/unix_util.h
)

set(LIBBOINC_HEADERS_APPLE
    ${BOINC_DIR}/lib/mac/dyld_gdb.h
    ${BOINC_DIR}/lib/mac/mac_backtrace.h
    ${BOINC_DIR}/lib/mac/mac_spawn.h
    ${BOINC_DIR}/lib/mac/QBacktrace.h
    ${BOINC_DIR}/lib/mac/QCrashReport.h
    ${BOINC_DIR}/lib/mac/QMachOImage.h
    ${BOINC_DIR}/lib/mac/QMachOImageList.h
    ${BOINC_DIR}/lib/mac/QSymbols.h
    ${BOINC_DIR}/lib/mac/QTaskMemory.h
    ${BOINC_DIR}/lib/mac/mac_branding.h
)

set(LIBBOINC_HEADERS_GENERIC
    ${BOINC_DIR}/lib/app_ipc.h
    ${BOINC_DIR}/lib/base64.h
    ${BOINC_DIR}/lib/boinc_stdio.h
    ${BOINC_DIR}/lib/cal_boinc.h
    ${BOINC_DIR}/lib/cc_config.h
    ${BOINC_DIR}/lib/cert_sig.h
    ${BOINC_DIR}/lib/cl_boinc.h
    ${BOINC_DIR}/lib/common_defs.h
    ${BOINC_DIR}/lib/coproc.h
    ${BOINC_DIR}/lib/crypt.h
    ${BOINC_DIR}/lib/diagnostics.h
    ${BOINC_DIR}/lib/error_numbers.h
    ${BOINC_DIR}/lib/filesys.h
    ${BOINC_DIR}/lib/hostinfo.h
    ${BOINC_DIR}/lib/keyword.h
    ${BOINC_DIR}/lib/md5.h
    ${BOINC_DIR}/lib/md5_file.h
    ${BOINC_DIR}/lib/mem_usage.h
    ${BOINC_DIR}/lib/mfile.h
    ${BOINC_DIR}/lib/miofile.h
    ${BOINC_DIR}/lib/network.h
    ${BOINC_DIR}/lib/notice.h
    ${BOINC_DIR}/lib/opencl_boinc.h
    ${BOINC_DIR}/lib/parse.h
    ${BOINC_DIR}/lib/prefs.h
    ${BOINC_DIR}/lib/proc_control.h
    ${BOINC_DIR}/lib/procinfo.h
    ${BOINC_DIR}/lib/project_init.h
    ${BOINC_DIR}/lib/proxy_info.h
    ${BOINC_DIR}/lib/std_fixes.h
    ${BOINC_DIR}/lib/str_replace.h
    ${BOINC_DIR}/lib/str_util.h
    ${BOINC_DIR}/lib/url.h
    ${BOINC_DIR}/lib/util.h
    ${BOINC_DIR}/lib/win_util.h
    ${BOINC_DIR}/lib/wslinfo.h
)

set(LIBBOINC_SOURCES_WIN
    ${BOINC_DIR}/lib/boinc_win.cpp
    ${BOINC_DIR}/lib/diagnostics_win.cpp
    ${BOINC_DIR}/lib/idlemon_win.cpp
    ${BOINC_DIR}/lib/procinfo_win.cpp
    ${BOINC_DIR}/lib/stackwalker_win.cpp
    ${BOINC_DIR}/lib/win_util.cpp
)

set(LIBBOINC_SOURCES_UNIX
    ${BOINC_DIR}/lib/procinfo_unix.cpp
    ${BOINC_DIR}/lib/synch.cpp
    ${BOINC_DIR}/lib/unix_util.cpp
)

set(LIBBOINC_SOURCES_APPLE
    ${BOINC_DIR}/lib/procinfo_mac.cpp
    ${BOINC_DIR}/lib/mac/mac_backtrace.cpp
    ${BOINC_DIR}/lib/mac/mac_spawn.cpp
    ${BOINC_DIR}/lib/mac/QBacktrace.c
    ${BOINC_DIR}/lib/mac/QCrashReport.c
    ${BOINC_DIR}/lib/mac/QMachOImage.c
    ${BOINC_DIR}/lib/mac/QMachOImageList.c
    ${BOINC_DIR}/lib/mac/QSymbols.c
    ${BOINC_DIR}/lib/mac/QTaskMemory.c
    ${BOINC_DIR}/lib/mac/mac_branding.cpp
)

set(LIBBOINC_SOURCES_GENERIC
    ${BOINC_DIR}/lib/app_ipc.cpp
    ${BOINC_DIR}/lib/base64.cpp
    ${BOINC_DIR}/lib/cc_config.cpp
    ${BOINC_DIR}/lib/cert_sig.cpp
    ${BOINC_DIR}/lib/coproc.cpp
    ${BOINC_DIR}/lib/crypt.cpp
    ${BOINC_DIR}/lib/diagnostics.cpp
    ${BOINC_DIR}/lib/filesys.cpp
    ${BOINC_DIR}/lib/hostinfo.cpp
    ${BOINC_DIR}/lib/keyword.cpp
    ${BOINC_DIR}/lib/md5.cpp
    ${BOINC_DIR}/lib/md5_file.cpp
    ${BOINC_DIR}/lib/mem_usage.cpp
    ${BOINC_DIR}/lib/mfile.cpp
    ${BOINC_DIR}/lib/miofile.cpp
    ${BOINC_DIR}/lib/network.cpp
    ${BOINC_DIR}/lib/notice.cpp
    ${BOINC_DIR}/lib/opencl_boinc.cpp
    ${BOINC_DIR}/lib/parse.cpp
    ${BOINC_DIR}/lib/prefs.cpp
    ${BOINC_DIR}/lib/proc_control.cpp
    ${BOINC_DIR}/lib/procinfo.cpp
    ${BOINC_DIR}/lib/project_init.cpp
    ${BOINC_DIR}/lib/proxy_info.cpp
    ${BOINC_DIR}/lib/shmem.cpp
    ${BOINC_DIR}/lib/str_util.cpp
    ${BOINC_DIR}/lib/url.cpp
    ${BOINC_DIR}/lib/util.cpp
    ${BOINC_DIR}/lib/wslinfo.cpp
)

set(LIBBOINCAPI_HEADERS
    ${BOINC_DIR}/api/boinc_api.h
)

set(LIBBOINCAPI_SOURCES_WIN
    ${BOINC_DIR}/lib/boinc_win.cpp
)

set(LIBBOINCAPI_SOURCES_GENERIC
    ${BOINC_DIR}/api/boinc_api.cpp
)

if(APPLE)
    set(LIBBOINC_HEADERS ${LIBBOINC_HEADERS_GENERIC} ${LIBBOINC_HEADERS_APPLE})
    set(LIBBOINC_SOURCES ${LIBBOINC_SOURCES_GENERIC} ${LIBBOINC_SOURCES_APPLE})

    set(LIBBOINCAPI_SOURCES ${LIBBOINCAPI_SOURCES_GENERIC})
elseif(WIN32)
    set(LIBBOINC_HEADERS ${LIBBOINC_HEADERS_GENERIC} ${LIBBOINC_HEADERS_WIN})
    set(LIBBOINC_SOURCES ${LIBBOINC_SOURCES_GENERIC} ${LIBBOINC_SOURCES_WIN})

    set(LIBBOINCAPI_SOURCES ${LIBBOINCAPI_SOURCES_GENERIC} ${LIBBOINCAPI_SOURCES_WIN})
elseif(UNIX)
    set(LIBBOINC_HEADERS ${LIBBOINC_HEADERS_GENERIC} ${LIBBOINC_HEADERS_UNIX})
    set(LIBBOINC_SOURCES ${LIBBOINC_SOURCES_GENERIC} ${LIBBOINC_SOURCES_UNIX})

    set(LIBBOINCAPI_SOURCES ${LIBBOINCAPI_SOURCES_GENERIC})
endif()

add_library(boinc ${LIBBOINC_SOURCES})
add_library(boincapi ${LIBBOINCAPI_SOURCES})

if(WIN32 OR APPLE)
    set_target_properties(boinc PROPERTIES PREFIX "lib")
    set_target_properties(boinc PROPERTIES IMPORT_PREFIX "lib")

    set_target_properties(boincapi PROPERTIES PREFIX "lib")
    set_target_properties(boincapi PROPERTIES IMPORT_PREFIX "lib")
endif()

# currently this is for MinGW only
set(build_options "")
if(HAVE_STRCASECMP)
    list(APPEND build_options "-DHAVE_STRCASECMP")
endif()

if(APPLE)
    target_include_directories(boinc PUBLIC
                                $<BUILD_INTERFACE:${BOINC_DIR}/lib/mac>
                                $<BUILD_INTERFACE:${BOINC_DIR}/clientgui/mac>
    )

    target_include_directories(boincapi PUBLIC
                                $<BUILD_INTERFACE:${BOINC_DIR}/lib/mac>
                                $<BUILD_INTERFACE:${BOINC_DIR}/clientgui/mac>
    )
elseif(WIN32)
    target_compile_definitions(boinc PUBLIC
                                -D_CRT_SECURE_NO_WARNINGS
                                -DWIN32
                                -D_CONSOLE
                                ${build_options}
    )

    target_compile_definitions(boincapi PUBLIC
                                -D_CRT_SECURE_NO_WARNINGS
                                -DWIN32
                                -D_CONSOLE
                                ${build_options}
    )

    target_include_directories(boinc PUBLIC
                                $<BUILD_INTERFACE:${BOINC_DIR}/win_build>
    )

    target_include_directories(boincapi PUBLIC
                                $<BUILD_INTERFACE:${BOINC_DIR}/win_build>
    )
elseif(UNIX)
    execute_process(COMMAND ./configure --disable-server --disable-client --disable-manager)

    execute_process(COMMAND ${BOINC_DIR}/configure --disable-server --disable-client --disable-manager
        WORKING_DIRECTORY ${BOINC_DIR}
        ECHO_OUTPUT_VARIABLE
        ECHO_ERROR_VARIABLE
    )

    file(COPY ${BOINC_DIR}/config.h DESTINATION ${BOINC_DIR}/config-h)


    target_include_directories(boinc PUBLIC
                                $<BUILD_INTERFACE:${BOINC_DIR}/config-h>
    )

    target_include_directories(boincapi PUBLIC
                                $<BUILD_INTERFACE:${BOINC_DIR}/config-h>
    )
endif()

target_include_directories(boinc PUBLIC
                            $<BUILD_INTERFACE:${BOINC_DIR}>
                            $<BUILD_INTERFACE:${BOINC_DIR}/lib>
                            $<INSTALL_INTERFACE:include>
)

target_include_directories(boincapi PUBLIC
                            $<BUILD_INTERFACE:${BOINC_DIR}>
                            $<BUILD_INTERFACE:${BOINC_DIR}/lib>
                            $<INSTALL_INTERFACE:include>
)

target_link_libraries(boinc PRIVATE
                        OpenSSL::SSL
                        OpenSSL::Crypto
)

target_link_libraries(boincapi PRIVATE
                        boinc
)


include(GNUInstallDirs)

install(TARGETS boinc boincapi EXPORT boinc-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${LIBBOINC_HEADERS} ${LIBBOINCAPI_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/boinc
)

install(EXPORT boinc-config NAMESPACE BOINC::boinc::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/boinc
)


message(WARNING "This build is under development and may not work as expected")
