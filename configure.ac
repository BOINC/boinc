dnl -*- autoconf -*-

dnl $Id$

dnl NOTE: on milkyway.ssl and jill.ssl , need autoconf/automake et al from quarl's
dnl directory; the _autosetup script does this.

dnl not sure exactly what the minimum version is (but 2.13 won't work)
AC_PREREQ(2.57)

dnl Process this file with autoconf to produce a configure script.
AC_INIT(BOINC, 2.27)

echo "--- Configuring BOINC AC_PACKAGE_VERSION ---"

AC_CANONICAL_SYSTEM

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)
AM_INIT_AUTOMAKE(dist-zip)

AC_CONFIG_SRCDIR(lib/shmem.C)

AC_COPYRIGHT([
The contents of this file are subject to the BOINC Public License
Version 1.0 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://boinc.berkeley.edu/license_1.0.txt

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License. 

The Original Code is the Berkeley Open Infrastructure for Network Computing. 

The Initial Developer of the Original Code is the SETI@home project.
Portions created by the SETI@home project are Copyright (C) 2002
University of California at Berkeley. All Rights Reserved. 

Contributor(s):
])

AC_REVISION([$Revision$])

AC_SUBST(MAJOR_VERSION, `echo AC_PACKAGE_VERSION | sed 's/\..*//'`)
AC_SUBST(MINOR_VERSION, `echo AC_PACKAGE_VERSION | sed 's/.*\.0*//'`)

AC_DEFINE_UNQUOTED(MAJOR_VERSION, $MAJOR_VERSION, [Major part of version number])
AC_DEFINE_UNQUOTED(MINOR_VERSION, $MINOR_VERSION, [Minor part of version number])
AC_DEFINE_UNQUOTED(HOSTTYPE, "$host", [Host for this compilation])
AC_SUBST(CLIENT_BIN_FILENAME,[boinc_]AC_PACKAGE_VERSION[_$host${EXEEXT}])
AC_SUBST(TOP_BUILD_DIR, [`pwd`])
AC_SUBST(TOP_SOURCE_DIR, [`(cd \`dirname "$0"\` && pwd)`])

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LN_S

dnl Checks for libraries.
AC_CHECK_LIB(gen, main)
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(nsl, gethostbyaddr)
AC_CHECK_LIB(socket, main)
AC_CHECK_LIB(stdc++, main)
AC_CHECK_LIB(z, gzopen)
dnl check for pthread
ACX_PTHREAD(AC_DEFINE(HAVE_PTHREAD,1, [Have pthread]))
dnl check for GL library
SAH_GRX_LIBS
AC_CHECK_LIB(socket, setservent)
AC_CHECK_LIB(gen, strfind)
AC_CHECK_LIB(gen, strfind, [ AC_DEFINE(HAVE_STRFIND, 1, [Have strfind]) ] )
AC_CHECK_LIB(cygipc, shmget)
AC_CHECK_MYSQL

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(arpa/inet.h dirent.h fcntl.h malloc.h netdb.h netinet/in.h netinet/tcp.h signal.h strings.h sys/file.h sys/ipc.h sys/mount.h sys/resource.h sys/select.h sys/shm.h sys/socket.h sys/statvfs.h sys/statfs.h sys/swap.h sys/sysctl.h sys/systeminfo.h sys/time.h sys/types.h sys/utsname.h sys/vmmeter.h sys/wait.h unistd.h errno.h)
SAH_GRX_INCLUDES
AC_LANG_PUSH(C++)
SAH_HEADER_STDCXX
SAH_CHECK_NAMESPACES
AH_TEMPLATE([HAVE_STD_MIN],[Define to 1 if min is in namespace std::])
AH_TEMPLATE([HAVE_STD_MAX],[Define to 1 if max is in namespace std::])
AH_TEMPLATE([HAVE_STD_TRANSFORM],[Define to 1 if transform is in namespace std::
])
SAH_FUNCS_IN_NAMESPACE([['min(0,0)'] ['max(0,0)'] ['transform((char *)0,(char *)
0,(char *)0,(int(*)(int))malloc)']],std)
AC_LANG_POP
dnl TODO: use compiler to test these; there probably exists an autoconf macro already!
case "$target" in
*-linux-* | *-sun-*)
        AC_DEFINE(GETSOCKOPT_SOCKLEN_T, 1, [getsockopt uses socklen_t])
        ;;
*sysv5OpenUNIX8*)
        AC_DEFINE(GETSOCKOPT_SIZE_T, 1, [getsockopt uses size_t])
        ;;
esac

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_VPRINTF
AC_FUNC_WAIT3
AC_CHECK_FUNCS(gethostname gettimeofday mkdir select socket strstr uname lockf flock strftime setpriority wait4 strlcpy strlcat)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

# by default, create static binaries on linux.
[if [ "$target_os" = "linux-gnu" ]; then
    STATIC_FLAGS="-static"
fi
echo "checking static flags... ${STATIC_FLAGS:-(none)}"]
AC_SUBST(STATIC_FLAGS)

AC_CONFIG_FILES([Makefile
                 RSAEuro/source/Makefile
                 RSAEuro/Makefile
                 api/Makefile
                 apps/Makefile
                 client/Makefile
                 db/Makefile
                 lib/Makefile
                 sched/Makefile
                 tools/Makefile
                 test/Makefile
                 test/version.inc
                 py/Makefile
                 py/Boinc/Makefile
                 py/Boinc/version.py
                 py/Boinc/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 py/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 client/win/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 sched/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 tools/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 test/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 ])

AC_CONFIG_HEADER([config.h])

AC_OUTPUT

echo "--- Configured BOINC AC_PACKAGE_VERSION ---"

[
true || $TOP_SOURCE_DIR/test/test_sanity.py || (
    cat <<EOF

******************************************************************************
**  
** The framework sanity check failed.
**  
** If you only want to compile (and run) the client or compile (and run) the
** server, this may be okay.
**  
** If you are developing BOINC and want to run the automated tests, you need a
** MySQL server installed and running, with permission to create databases.
**
** You can run this sanity check using test/test_sanity.py
**
******************************************************************************

EOF

)
]
