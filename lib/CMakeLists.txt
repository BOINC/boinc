cmake_minimum_required(VERSION 3.10)

# set the project name
project(lib)

set(CMAKE_BUILD_TYPE Release)

find_package(PkgConfig       QUIET)

set(libfcgi_sources
    boinc_fcgi.cpp
    coproc.cpp
    filesys.cpp
    hostinfo.cpp
	keyword.cpp
    md5.cpp
    md5_file.cpp
    mfile.cpp
    miofile.cpp
    msg_log.cpp
    opencl_boinc.cpp
    parse.cpp
    sched_msgs.cpp
    shmem.cpp
    str_util.cpp
    synch.cpp
    url.cpp
    util.cpp
	wslinfo.cpp
)

set(generic_sources
    app_ipc.cpp
    base64.cpp
    cc_config.cpp
    cert_sig.cpp
    coproc.cpp
    diagnostics.cpp
    filesys.cpp
    gui_rpc_client.cpp
    gui_rpc_client_ops.cpp
    gui_rpc_client_print.cpp
    hostinfo.cpp
	keyword.cpp
    md5.cpp
    md5_file.cpp
    mem_usage.cpp
    mfile.cpp
    miofile.cpp
    msg_log.cpp
    network.cpp
    notice.cpp
    opencl_boinc.cpp
    parse.cpp
    prefs.cpp
    procinfo.cpp
    proc_control.cpp
    project_init.cpp
    proxy_info.cpp
    sched_msgs.cpp
    shmem.cpp
    str_util.cpp
    url.cpp
    util.cpp
	wslinfo.cpp
    procinfo_unix.cpp
    synch.cpp
    unix_util.cpp    
)

set(pkginclude_HEADERS
    app_ipc.h
    average.h
    base64.h
    boinc_fcgi.h
    cal_boinc.h
    cc_config.h
    cert_sig.h
    cl_boinc.h
    common_defs.h
    coproc.h
    crypt.h
    diagnostics.h
    error_numbers.h
    filesys.h
    gui_rpc_client.h
    hostinfo.h
    md5.h
    md5_file.h
    mem_usage.h
    mfile.h
    miofile.h
    msg_log.h
    network.h
    notice.h
    opencl_boinc.h
    parse.h
    prefs.h
    procinfo.h
    proxy_info.h
    sched_msgs.h
    stackwalker_imports.h
    str_util.h
    url.h
    util.h
)

set(noinst_HEADERS
    msg_queue.h
    proc_control.h
    procinfo.h
    shmem.h
    str_replace.h
    synch.h
    unix_util.h
)

include_directories(
    .
    ..
)

add_library(libboinc.la STATIC ${generic_sources} )

SOURCE_GROUP("Source Files" FILES ${LIB_SOURCE})
SOURCE_GROUP("Header Files" FILES ${LIB_HEADER})        