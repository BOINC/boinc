dnl This file is part of BOINC.
dnl https://boinc.berkeley.edu
dnl Copyright (C) 2025 University of California
dnl
dnl BOINC is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU Lesser General Public License
dnl as published by the Free Software Foundation,
dnl either version 3 of the License, or (at your option) any later version.
dnl
dnl BOINC is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
dnl See the GNU Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
dnl

dnl -*- autoconf -*-

dnl $Id$

dnl not sure exactly what the minimum version is (but 2.13 wont work)
AC_PREREQ(2.58)

dnl Set the BOINC version here.  You can also use the set-version script.
AC_INIT(BOINC, 8.3.0)
AC_CONFIG_MACRO_DIR([m4])
LIBBOINC_VERSION=`echo ${PACKAGE_VERSION} | sed 's/\./:/g'`
AC_SUBST([LIBBOINC_VERSION])

WRAPPER_RELEASE=26019
AC_SUBST([WRAPPER_RELEASE])

VBOXWRAPPER_RELEASE=26211
AC_SUBST([VBOXWRAPPER_RELEASE])

WORKER_RELEASE=6
AC_SUBST([WORKER_RELEASE])

DOCKERWRAPPER_RELEASE=15
AC_SUBST([DOCKERWRAPPER_RELEASE])

MULTITHREAD_RELEASE=2
AC_SUBST([MULTITHREAD_RELEASE])

AC_CANONICAL_TARGET

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)

dnl subdir-objects is supposed to become the default soon, but specifying it
dnl breaks depfile generation.  We'll worry about it when that happens.
dnl AM_INIT_AUTOMAKE(dist-zip subdir-objects)

AM_INIT_AUTOMAKE(dist-zip)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR(lib/shmem.cpp)

AC_REVISION([$Revision$])
REV=`echo '$Revision$' | awk "{print $2}"`
RDATE=`date '+%Y.%m.%d'`
AC_SUBST(REV)
AC_SUBST(RDATE)

dnl Are we compiling for windows
if echo $host_os | grep -E '^mingw|^winnt' > /dev/null ; then
  isWIN32=yes
else
  isWIN32=no
fi

dnl turn dependency tracking on by default
if test x${enable_dependency_tracking} != xno ; then
  enable_dependency_tracking=yes
fi

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX

dnl Verify C++11 support
AC_MSG_CHECKING([whether $CXX supports C++11])
AC_LANG_PUSH([C++])
save_CXXFLAGS="$CXXFLAGS"
for flag in "-std=c++11" "-std=c++0x"; do
  CXXFLAGS="$save_CXXFLAGS $flag"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <memory>
    #include <array>
    #include <functional>
    ]],
    [[
    auto ptr = std::make_shared<int>(42);
    std::array<int, 5> arr = {{1, 2, 3, 4, 5}};
    auto lambda = [](int x) { return x * 2; };
    return lambda(*ptr) + arr[0];
    ]])],
    [have_cxx11=yes],
    [have_cxx11=no])
  if test "x$have_cxx11" = "xyes"; then
    AC_MSG_RESULT([yes, with $flag])
    CXXFLAGS="$save_CXXFLAGS $flag"
    break
  fi
done
if test "x$have_cxx11" != "xyes"; then
  CXXFLAGS="$save_CXXFLAGS"
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([
================================================================================
ERROR: A C++11 compliant compiler is required to build BOINC.
Please use a newer compiler version (e.g., GCC 4.8+, Clang 3.3+).
================================================================================
  ])
fi
AC_LANG_POP([C++])

dnl ------
dnl Workaround for autoconf >= 2.65 backwards incompatibility
m4_pattern_allow([AC_PROG_OBJCXX])
m4_ifdef([AC_PROG_OBJCXX],[AC_PROG_OBJCXX],)
dnl ------
AC_PROG_CPP
AC_PROG_MAKE_SET
SAH_LINKS
AC_LANG_PUSH(C)
AM_PROG_CC_C_O

PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

m4_divert_once([HELP_ENABLE],
  AS_HELP_STRING([BOINC Default enable values], [--enable-server --enable-client --enable-libraries --enable-manager: builds server, client, and libraries]))

AC_ARG_ENABLE(dynamic-client-linkage,
    AS_HELP_STRING([--enable-dynamic-client-linkage],
                   [dynamically link the client to the boinc libraries]),
    [dynamic_client=${enableval}],
    [dynamic_client=no])

AC_ARG_ENABLE(server,
    AS_HELP_STRING([--disable-server],
                   [disable building the scheduling server]),
    [enable_server=${enableval}],
    [enable_server=yes])

AC_ARG_ENABLE(fcgi,
    AS_HELP_STRING([--disable-fcgi],
                   [disable building fcgi libraries or server components]),
    [enable_fcgi=${enableval}],
    [enable_fcgi=${enable_server}])

AC_ARG_ENABLE(client,
    AS_HELP_STRING([--disable-client],
                   [disable building the client]),
    [enable_client=${enableval}],
    [enable_client=yes])

AC_ARG_ENABLE(manager,
    AS_HELP_STRING([--disable-manager],
                   [disable building the boinc manager gui]),
    [enable_manager=${enableval}],
    [enable_manager=yes])

AC_ARG_ENABLE(libraries,
    AS_HELP_STRING([--disable-libraries],
                   [disable building the boinc client and server libraries]),
    [enable_libraries=${enableval}],
    [enable_libraries=yes])

AC_ARG_ENABLE(install-headers,
    AS_HELP_STRING([--disable-install-headers],
                   [if building libraries do not install the boinc library
                    header files]),
    [enable_install_headers=${enableval}],
    [enable_install_headers=yes])

AC_ARG_ENABLE(boinczip,
    AS_HELP_STRING([--enable-boinczip],
                   [enable building the boinc zip library]),
    [enable_boinczip=${enableval}],
    [enable_boinczip=no])

AC_ARG_ENABLE(apps,
    AS_HELP_STRING([--enable-apps],
                   [enable building the boinc example apps]),
    [enable_apps=${enableval}],
    [enable_apps=no])

AC_ARG_ENABLE(apps-vcpkg,
    AS_HELP_STRING([--enable-apps-vcpkg],
                   [enable building the boinc apps that depend on vcpkg]),
    [enable_apps_vcpkg=${enableval}],
    [enable_apps_vcpkg=no])

AC_ARG_ENABLE(apps-mingw,
    AS_HELP_STRING([--enable-apps-mingw],
                   [enable building the boinc apps with mingw]),
    [enable_apps_mingw=${enableval}],
    [enable_apps_mingw=no])

AC_ARG_ENABLE(apps-vbox,
    AS_HELP_STRING([--enable-apps-vbox],
                   [enable building the boinc apps that depend on vbox]),
    [enable_apps_vbox=${enableval}],
    [enable_apps_vbox=no])

AC_ARG_ENABLE(apps-gui,
    AS_HELP_STRING([--enable-apps-gui],
                   [enable building the boinc apps that depend on gui]),
    [enable_apps_gui=${enableval}],
    [enable_apps_gui=no])

AC_ARG_ENABLE(unit-tests,
    AS_HELP_STRING([--enable-unit-tests],
                   [enable building the boinc unit tests]),
    [enable_unit_tests=${enableval}],
    [enable_unit_tests=no])

AC_ARG_ENABLE(pkg-libs,
    AS_HELP_STRING([--enable-pkg-libs],
                   [Builds and installs components that would be present in a
		   boinc-libs package.  This is a synonym for --disable-server
		   --disable-client --enable-libraries --disable-static
		   --disable-install-headers --disable-manager --enable-boinczip]),
    [
     enable_libraries=yes
     enable_server=no
     enable_client=no
     enable_manager=no
     enable_install_headers=no
     enable_static=no
     enable_boinczip=yes
     enable_apps=no
     enable_unit_tests=no
    ],
    [])

AC_ARG_ENABLE(pkg-devel,
    AS_HELP_STRING([--enable-pkg-devel],
                   [Builds and installs components that would be present in a
		   boinc-devel package.  This is a synonym for --disable-server
		   --disable-client --enable-libraries --enable-install-headers
		   --disable-manger --enable-boinczip]),
    [
     enable_libraries=yes
     enable_server=no
     enable_client=no
     enable_manager=no
     enable_install_headers=yes
     enable_boinczip=yes
     enable_apps=no
     enable_unit_tests=no
    ],
    [])

AC_ARG_ENABLE(pkg-client,
    AS_HELP_STRING([--enable-pkg-client],
                   [Builds and installs components that would be present in a
		   boinc-client package. This is a synonym for --disable-server
		   --enable-client --disable-libraries --disable-manager
    ]),
    [
     enable_libraries=no
     enable_server=no
     enable_client=yes
     enable_manager=no
     enable_install_headers=no
     enable_boinczip=no
     enable_apps=no
     enable_unit_tests=no
    ],
    [])

AC_ARG_ENABLE(pkg-manager,
    AS_HELP_STRING([--enable-pkg-manager],
                   [Builds and installs components that would be present in a boinc-manager package. This is a synonym for --disable-server --disable-client --disable-libraries --enable-manager
    ]),
    [
     enable_libraries=no
     enable_server=no
     enable_client=no
     enable_manager=yes
     enable_install_headers=no
     enable_boinczip=no
     enable_apps=no
     enable_unit_tests=no
    ],
    [])

BOINC_SET_COMPILE_FLAGS

configured_to_build=

if test x$enable_client = xyes ; then
  configured_to_build="client"
  enable_xss="yes"
fi
if test x$enable_manager = xyes ; then
  configured_to_build="${configured_to_build} manager"
fi
if test x$enable_libraries = xyes ; then
  configured_to_build="${configured_to_build} libraries"
fi
if test x$enable_server = xyes ; then
  configured_to_build="${configured_to_build} server"
fi
if test x$enable_apps = xyes ; then
  configured_to_build="${configured_to_build} apps"
fi
if test x$enable_unit_tests = xyes ; then
  configured_to_build="${configured_to_build} unit-tests"
fi

if test -z "${configured_to_build}" ; then
        AC_MSG_ERROR([
================================================================================
ERROR: I have been told to build nothing.

    Please remove one of the --disable commands from your
    configure command line.

================================================================================
        ])
fi


dnl ======================================================================
dnl some vodoo required for building portable client-binary (client, clientgui)
dnl ======================================================================
AC_ARG_ENABLE([client-release],
   AS_HELP_STRING([--enable-client-release],
        [Attempt to build a portable "release-candidate": this links libstd++ and
	other addon-libraries statically.
	(On GNU/Linux you might need to use gcc-3.0 for this to produce a
	binary that works for glibc <= 2.2 systems. => use CC=gcc-3.0 and CXX=g++-3.0)
	(Default = no)]),
    [
       enable_client_release=yes
       disable_static_linkage=no
       enable_shared=no
       AC_MSG_WARN([
================================================================================
WARNING:  You have requested a statically linked release build.

    This option is not recommended for personal use or for building
    an application for distribution via a package management system.
    It probably wont work unless you have built your own static curl
    and wxWidgets libraries that have minimal dependencies.  You may
    also need to edit Makefiles manually in order for linking to work.

    It is far better to use dynamic linkage, which can resolve most
    library interdependencies at run time.

    When the executable fails to link, and you can identify the missing
    library, you can edit the Makefile to add it, or if there is a --with
    option to configure, use it.

    This is not easy.  It is not supported.  You have been warned.
================================================================================
    ])
    echo Pausing 10 seconds...
    sleep 10
    ]
    ,
    [enable_client_release=no;disable_static_linkage=yes])


if test "${enable_debug}" = yes ; then
    build_state='Debug'
else
    build_state='Release'
fi

echo "--- Configuring BOINC AC_PACKAGE_VERSION (${build_state}) ---"
echo "--- Build Components: (${configured_to_build}) ---"


AC_COPYRIGHT([
Berkeley Open Infrastructure for Network Computing
http://boinc.berkeley.edu
Copyright (C) 2019 University of California

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
either version 2.1 of the License, or (at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

To view the GNU Lesser General Public License visit
http://www.gnu.org/copyleft/lesser.html
or write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
])

AC_DEFUN([KC_STRIP_LEADING_ZEROS],[m4_bregexp([$*],[^0*\(..*\)],\1)])

AC_DEFUN([BOINC_SET_VERSION],[dnl
AC_SUBST([BOINC_MAJOR_VERSION],KC_STRIP_LEADING_ZEROS([$1]))
AC_SUBST([BOINC_MINOR_VERSION],KC_STRIP_LEADING_ZEROS([$2]))
AC_SUBST([BOINC_RELEASE],KC_STRIP_LEADING_ZEROS([$3]))
AC_SUBST([BOINC_VERSION_STRING],AC_PACKAGE_VERSION)
])

BOINC_SET_VERSION(translit(AC_PACKAGE_VERSION, [.], [,]))

dnl Version information moved to version.h so removed from config.h
dnl AC_DEFINE_UNQUOTED([BOINC_VERSION_STRING], "$BOINC_VERSION_STRING", [String representation of BOINC version number])
dnl AC_DEFINE_UNQUOTED([BOINC_MAJOR_VERSION], $BOINC_MAJOR_VERSION, [Major part of BOINC version number])
dnl AC_DEFINE_UNQUOTED([BOINC_MINOR_VERSION], $BOINC_MINOR_VERSION, [Minor part of BOINC version number])
dnl AC_DEFINE_UNQUOTED([BOINC_RELEASE], $BOINC_RELEASE, [Release part of BOINC version number])

dnl Need to duplicate these AC_SUBST because some versions of autoconf wont
dnl find AC_SUBST in a macro unless subsequently used in a DEFINE or SUBST
dnl outside of a macro.  Dont ask me why.
AC_SUBST([BOINC_VERSION_STRING])
AC_SUBST([BOINC_MAJOR_VERSION])
AC_SUBST([BOINC_MINOR_VERSION])

AC_SUBST([TOP_BUILD_DIR], [`pwd`])

## commented this out to turn all maintainer-mode features ON by default,
## such as automatic reconfiguring on changes in Makefile.am's and configure.ac
## AM_MAINTAINER_MODE


dnl Check for docbook2x-man, which we use to create the manpages in
dnl doc/manpages, and set the conditional HAVE_DOCBOOK2X_MAN to build
dnl the manpages only if docbook2x-man is available.
AC_PATH_PROG(DOCBOOK2X_MAN, docbook2x-man)
AM_CONDITIONAL(HAVE_DOCBOOK2X_MAN, [test -n "${DOCBOOK2X_MAN}"])

AC_SUBST([CLIENT_BIN_FILENAME],[boinc${EXEEXT}])
AC_SUBST([CLIENT_CMD_BIN_FILENAME],[boinccmd${EXEEXT}])
AC_SUBST([CLIENT_GUI_BIN_FILENAME],[boincmgr${EXEEXT}])

dnl If we're running under cygwin, check whether we want to use X11 or WIN32
dnl GUI
BOINC_CHECK_CYGWIN

dnl enable objective-C
if test x$OBJC = x ; then
  OBJC="$CC"
fi
if test x$OBJCXX = x ; then
  OBJCXX="$CXX"
fi
_AM_DEPENDENCIES([OBJC])
AC_SUBST(OBJC)
AC_SUBST(OBJCXX)
OBJCFLAGS="-ObjC"
AC_SUBST(OBJCFLAGS)

dnl LT_INIT
enable_shared_user=$enable_shared
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_SUBST(PICFLAGS,${lt_prog_compiler_pic})
if test "$enable_shared_user" != no -a "$enable_shared" = no ; then
  AC_MSG_ERROR([Project is configured to produce shared libraries, but compiler doesn't support shared libraries creation.])
fi

SAH_OPTION_BITNESS

dnl Determine the BOINC platform given the target arch-platform-os.
BOINC_PLATFORM

if test "${enable_client_release}" = yes -a "${enable_client}" != yes; then
   AC_MSG_WARN([--enable-client-release ignored.
--------------------------------------------------
The switch --enable-client-release is only useful for building the client and will be ignored
--------------------------------------------------])
fi



dnl Find the extensions used on shared and static libraries on this system
SAH_LIBEXT
SAH_DLLEXT

dnl THIS IS UNNECESSARY.  THE COMPILER SHOULD SET THESE
dnl enable debug mode on all components using standard debug flags
dnl if test "${enable_debug}" = yes; then
    dnl CXXFLAGS="$CXXFLAGS -DDEBUG -D_DEBUG"
    dnl CFLAGS="$CFLAGS -DDEBUG -D_DEBUG"
dnl
    dnl AC_SUBST(CXXFLAGS)
    dnl AC_SUBST(CFLAGS)
dnl fi


if test "${enable_server}" = yes; then
  AC_CHECK_MYSQL
  if test "${no_mysql}" = yes; then
    AC_MSG_ERROR([
================================================================================
ERROR: trying to build BOINC-server, but MYSQL was not found.

If you do not want to build the server you should use --disable-server.

================================================================================
    ])
  fi

   dnl Check for nlohmann::json (header-only library)
   AC_ARG_WITH([nlohmann_json],
     [AS_HELP_STRING([--with-nlohmann_json=DIR], [Specify prefix for nlohmann::json (header-only) library])],
     [nlohmann_json_dir="$withval"],
     [nlohmann_json_dir=""])

   if test "x$nlohmann_json_dir" != "x"; then
     CPPFLAGS="$CPPFLAGS -I$nlohmann_json_dir/include"
   fi

   dnl Ensure we use C++ and require C++11 for the header check
   AC_LANG_PUSH([C++])
   m4_ifdef([AX_CXX_COMPILE_STDCXX], [AX_CXX_COMPILE_STDCXX(11)])
   AC_CHECK_HEADER([nlohmann/json.hpp],
     [AC_MSG_RESULT([Found nlohmann::json])],
     [AC_MSG_ERROR([
================================================================================
ERROR: nlohmann::json (nlohmann/json.hpp) is required to build the server but was not found.

Please install it (e.g., via your package manager or from https://github.com/nlohmann/json),
or specify its location with --with-nlohmann_json=DIR.
================================================================================
     ])])
   AC_LANG_POP([C++])
fi



dnl Checks for libraries.

dnl NOTE: Libraries subject to version changes, or those that might not be
dnl installed everywhere should be linked statically when building portable
dnl binaries.  Put libraries that should be linked statically on every platform
dnl in the STATIC_LIB_LIST variable before calling SAH_CHECK_LIB.  (Note: Only
dnl the '*' wildcard is supported).
STATIC_LIB_LIST="stdc++* gcc gcc_s gcc_eh atk* gtk* gdk* gmodule* gthread* gobject* glib* pango* cairo* wx* tiff* jpeg* png* Xi glut socket z expat* krb5support freetype fontconfig* resolv* idn c_nonshared gpg* gss* gnutls sasl2"

dnl We'll store these libs in a separate variable BOINC_EXTRA_LIBS for later
dnl post-processing instead of the default-action, which would append
dnl them to LIBS (note, we also dont define HAVE_LIBXXX, because
dnl currently they arent used)

dnl Special rules for specific targets go here:  It's easier to add static libs
dnl than subtract them.  If you need to remove something from the list above,
dnl please add it to the lists below...
case ${target} in
	*-linux*|*-k*bsd*-gnu)	STATIC_LIB_LIST="${STATIC_LIB_LIST} nsl"
	;;
	*-solaris*)	STATIC_LIB_LIST="${STATIC_LIB_LIST} X* ssl crypto"
	;;
esac

if test "x${enable_fcgi}" = xyes ; then
dnl m4/boinc_check_fcgi.m4
BOINC_CHECK_FCGI
fi
dnl ---------- SSL (m4/check_ssl.m4)
AS_IF([test "x${enable_client}" = "xyes" -o "x${enable_server}" = "xyes"],
  [CHECK_SSL])

dnl ---------- libcurl (m4/libcurl.m4) ------------------------------
dnl curl is needed for client

AS_IF([test "x${enable_client}" = "xyes" -o "x${enable_server}" = "xyes"],
  [
    LIBCURL_CHECK_CONFIG([yes], [7.17.1], [haveCurl=yes], [haveCurl=no])

    AS_IF([test "${haveCurl}" != "yes"],
      [
        AC_MSG_ERROR([
================================================================================
ERROR: could not find (recent enough) development-libs for libcurl.

  If libcurl-dev is installed on your system, make sure that the script
  'curl-config' is found in your PATH, and that
  'curl-config --version' gives something recent enough (see above).

  You can download libcurl from: http://curl.haxx.se/

================================================================================
        ])
      ],
      [
        ## add libcurl et al. to the list of statically linked libs
        STATIC_LIB_LIST="${STATIC_LIB_LIST} curl idn ssh2 crypto ssl krb5 k5crypto gssapi_krb5 com_err resolv lber ldap socket nsl z rt gcrypt gpg-error"
        CPPFLAGS="${CPPFLAGS} ${LIBCURL_CPPFLAGS}"
        CURL_LIB_PATHS=`echo $LIBCURL | sed 's/[^[a-zA-Z]]*-l[^ ]*//g'`

        AS_IF([test "${enable_debug}" = "yes"],
          [
            echo "LIBCURL = ${LIBCURL}"
            echo "LIBCURL_CPPFLAGS = ${LIBCURL_CPPFLAGS}"
            echo "CURL_LIB_PATHS = ${CURL_LIB_PATHS}"
            echo "LDFLAGS = ${LDFLAGS}"
          ])

        BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${LIBCURL}"
      ])
  ])

if test "x${found_ssl}" = "xyes"; then
   BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${SSL_LIBS}"
fi

dnl save the libs, since SAH_CHECK_LIB() will change them
SAVELIBS_AAF="${LIBS}"
SAH_CHECK_LIB([cygipc], [shmget],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([aio], [aio_fork],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([dl], [dlopen],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([nsl], [gethostbyname],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([freetype], [fopen],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([socket], [bind],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([z], [gzopen], [
      zlib_found=yes
      BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
if test "x${zlib_found}" = "x" -a "${isWIN32}" = "no"; then
    if test "x${enable_client}" = "xyes" -o "x${enable_server}" = "xyes" -o "x${enable_apps_vbox}" = "xyes"]; then
        AC_MSG_ERROR([zlib is required but was not found])
    fi
fi
SAH_CHECK_LIB([cups], [md5_finish],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
if test "${isWIN32}" = "yes" ; then
   SAH_CHECK_LIB([wsock32], [fopen],
       [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
   SAH_CHECK_LIB([wsock], [fopen],
       [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
   SAH_CHECK_LIB([comctl32], [fopen],
       [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
   SAH_CHECK_LIB([winmm], [fopen],
       [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
   SAH_CHECK_LIB([msimg32], [fopen],
       [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
fi
if test "x${enable_manager}" = "xyes" -a "${isWIN32}" = "no"; then
    SAH_CHECK_LIB([xkbcommon], [xkb_state_key_get_utf8],
       [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
fi

LIBS=${SAVELIBS_AAF}

dnl check for pthread
AX_PTHREAD(AC_DEFINE(HAVE_PTHREAD,1, [Have pthread]))
dnl There are better ways to do this.
dnl CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
dnl CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
dnl LIBS="$PTHREAD_LIBS $LIBS"

# find the install directories in case we need them.
if test "x${prefix}" = "xNONE" -o -z "${prefix}" ; then
  prefix="${ac_default_prefix}"
fi
if test "x${exec_prefix}" = "xNONE" -o -z "${exec_prefix}" ; then
  exec_prefix="${prefix}"
fi
if test "x${bindir}" = "xNONE" -o -z "${bindir}" ; then
  bindir="${exec_prefix}/bin"
fi
if test "x${libdir}" = "xNONE" -o -z "${libdir}" ; then
  bindir="${exec_prefix}/lib"
fi

AC_DEFINE_UNQUOTED(BOINC_PREFIX,"`eval echo ${prefix}`",[Directory tree where BOINC will be installed])
AC_DEFINE_UNQUOTED(BOINC_EXECPREFIX,"`eval echo ${bindir}`",[Directory where BOINC executables will be installed])
AC_DEFINE_UNQUOTED(BOINC_LIBDIR,"`eval echo ${libdir}`",[Directory where BOINC libraries will be installed])

dnl ---------- decide wether to build the graphical boinc-api libs or not
dnl check for X11
AC_PATH_X
AC_PATH_XTRA

dnl Check for CL library opencl
AX_CHECK_CL
if test "x$with_opencl" = "xno"; then
  AC_MSG_WARN([OpenCL is missing, OpenClApp example is disabled])
fi

dnl check for GL library
AX_CHECK_GL
AX_CHECK_GLU

dnl check for glut and prerequesites to the glut-test: libXmu and libXi
dnl or on windows libGDI32 and libWinMM
AC_CHECK_LIB([Xmu], [fopen], [have_Xmu="yes"], [have_Xmu="no"])
AC_CHECK_LIB([Xi], [fopen], [have_Xi="yes"], [have_Xi="no"])
if test "${isWIN32}" = "yes" ; then
  AC_CHECK_LIB([gdi32], [fopen])
  AC_CHECK_LIB([winmm], [fopen])
fi

AX_CHECK_GLUT
if test "X${no_x}" != "Xyes" -a "$no_glut" = yes; then
   if test "$have_Xmu" = no; then
   	AC_MSG_WARN([Test for glut-dev might have failed because of missing libXmu-dev!])
   fi
   if test "$have_Xi" = no; then
   	AC_MSG_WARN([Test for glut-dev might have failed because of missing libXi-dev!])
   fi
fi

if test "$no_gl" = yes -o "$no_glu" = yes -o "$no_glut" = yes; then
   have_glut=no
   AC_MSG_WARN([
================================================================================
WARNING: Development libraries and headers ("-dev") of {openGL, GLU, glut} needed!

The GL, GLU and glut libraries are required in order to build the graphical parts
of the BOINC application API library.

==> only building non-graphical parts of the BOINC API Library for now.

HINT: on MacOS X/Darwin you might consider running configure with the option
      './configure --with-apple-opengl-framework'
      in order to use the Mac-native openGL framework

================================================================================
       ])
else
   have_glut=yes
   echo "DEBUG: GLUT_CFLAGS = $GLUT_CFLAGS" >&5
   echo "DEBUG: GLUT_LIBS = $GLUT_LIBS" >&5

   AC_CHECK_HEADERS([gl.h glu.h glut.h glaux.h GL/gl.h GL/glu.h GL/glut.h GL/glaux.h OpenGL/gl.h OpenGL/glu.h OpenGL/glut.h OpenGL/glaux.h GLUT/glut.h MesaGL/gl.h MesaGL/glu.h MesaGL/glut.h MesaGL/glaux.h libnotify/notify.h gtk/gtk.h])

   AC_CHECK_LIB([jpeg], [jpeg_start_compress],[have_jpeg=1],[have_jpeg=0])
   AC_CHECK_HEADER([jpeglib.h],[have_jpeg=1],[have_jpeg=0])

   if test "$have_jpeg" != 1 ; then
	AC_MSG_WARN([
================================================================================
WARNING: libjpeg.a/jpeglib.h not found.

BOINC's openGL graphics-API needs libjpeg !
see http://www.ijg.org/

==> I will continue building the non-graphical parts of the BOINC API library.

HINT: on MacOSX/Darwin these libs/includes are sometimes found in '/sw/' (if using fink)
      or '/opt/local' (if using port).
================================================================================
	]);
   fi
fi

AM_CONDITIONAL(BUILD_GRAPHICS_API, [ test "$have_glut" = yes -a "$have_jpeg" = 1])

dnl check for X screen saver lib (X-based idle detection on Linux)
if test "$enable_xss" = yes; then
    if test "X${no_x}" = "Xyes" ; then
        AC_MSG_WARN([X Window include files/libs excluded or location unknown, disabling X ScreenSaver user idle detection])
    else
        AC_CHECK_LIB([Xss], [XScreenSaverAllocInfo], [have_Xss="yes"], [have_Xss="no"])
        AC_CHECK_HEADER([X11/extensions/scrnsaver.h], [have_Xss="yes"], [have_Xss="no"])
        if test "$have_Xss" = no; then
            AC_MSG_WARN([libxss missing, disabling X ScreenSaver user idle detection])
        fi
    fi
fi

dnl --------------------------------------------------------------------------------
dnl put double-inclusion protection into config.h
AH_TOP([
/* double-inclusion protection for config.h */
#ifndef BOINC_CONFIG_H
#define BOINC_CONFIG_H

/* Version defines are now in version.h */
#include "version.h"

])
AH_BOTTOM([

#ifndef HAVE_RES_INIT
#define res_init() (0)
#endif

#include "project_specific_defines.h"

/* end double-inclusion protection for config.h */
#endif /* #ifndef BOINC_CONFIG_H */
])
dnl ----------------------------------------------------------------------

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_TYPE_SIGNAL
if test "${isWIN32}" = "yes" ; then
  AC_CHECK_HEADERS(winsock2.h winsock.h windows.h ws2tcpip.h winternl.h crtdbg.h)
fi
AC_CHECK_HEADERS(sys/types.h sys/un.h arpa/inet.h dirent.h grp.h fcntl.h inttypes.h stdint.h memory.h netdb.h netinet/in.h netinet/tcp.h netinet/ether.h net/if.h net/if_arp.h signal.h strings.h sys/auxv.h sys/file.h sys/fcntl.h sys/ipc.h sys/ioctl.h sys/msg.h sys/param.h sys/resource.h sys/select.h sys/sem.h sys/shm.h sys/sockio.h sys/socket.h sys/stat.h sys/statvfs.h sys/statfs.h sys/systeminfo.h sys/time.h sys/types.h sys/utsname.h sys/vmmeter.h sys/wait.h unistd.h utmp.h errno.h procfs.h ieeefp.h setjmp.h float.h sal.h execinfo.h xlocale.h)

save_cxxflags="${CXXFLAGS}"
save_cppflags="${CPPFLAGS}"
sse3_flags="-msse3"
avx_flags="-mavx"
CXXFLAGS="${save_cxxflags} ${sse3_flags}"
CPPFLAGS="${save_cppflags} ${sse3_flags}"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],)], [], [sse_flags=""])
CXXFLAGS="${save_cxxflags} ${avx_flags}"
CPPFLAGS="${save_cppflags} ${avx_flags}"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],)], [], [avx_flags=""])
CXXFLAGS="${save_cxxflags} ${sse3_flags}"
CXXFLAGS="${save_cxxflags} ${sse3_flags}"
AC_CHECK_HEADERS([intrin.h x86intrin.h pmmintrin.h xmmintrin.h emmintrin.h])
CXXFLAGS="${save_cxxflags} ${avx_flags}"
CPPFLAGS="${save_cppflags} ${avx_flags}"
AC_CHECK_HEADERS([immintrin.h])

AC_CHECK_DECLS([_xgetbv, xgetbv, __xgetbv, cpuid, _cpuid, __cpuid],
    [],[],[[
#if HAVE_INTRIN_H
# include <intrin.h>
#endif
#if HAVE_IMMINTRIN_H
# include <immintrin.h>
#endif
#if HAVE_X86INTRIN_H
# include <x86intrin.h>
#endif
#if HAVE_PMMINTRIN_H
# include <pmmintrin.h>
#endif
#if HAVE_XMMINTRIN_H
# include <xmmintrin.h>
#endif
#if HAVE_EMMINTRIN_H
# include <emmintrin.h>
#endif
]])

AC_MSG_CHECKING(if assembler supports xgetbv)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(,[asm ("xgetbv");])],
  AC_DEFINE([ASM_SUPPORTS_XGETBV],1,[Define to 1 if the xgetbv instruction can be used in inline assember])
  AC_MSG_RESULT(yes),
  AC_MSG_RESULT(no)
)

CXXFLAGS="${save_cxxflags}"
CPPFLAGS="${save_cppflags}"

if test "${isWIN32}" = "yes" ; then
  AC_CHECK_HEADERS([winhttp.h ntapi.h ddk/ntapi.h dbghelp.h delayimp.h],[], [], [[
  #if HAVE_WINDOWS_H
  # include <windows.h>
  #endif
  ]])

  AC_CHECK_HEADER(security.h, [
  AC_DEFINE(HAVE_SECURITY_H,1,[Define to 1 if your compiler has the security.h header file])
  AC_DEFINE(SECURITY_WIN32,1,[Define to 1 if your compiler has the security.h header file])
  ], [], [[
  #define SECURITY_WIN32 1
  #if HAVE_WINDOWS_H
  # include <windows.h>
  #endif
  ]])

  AC_CHECK_TYPES([SYSTEM_PROCESSES, CLIENT_ID, VM_COUNTERS, SYSTEM_THREADS, THREAD_STATE, THREAD_WAIT_REASON],[],[],[[
  #if HAVE_WINDOWS_H
  #include <windows.h>
  #endif
  #ifdef HAVE_WINTERNL_H
  #include <winternl.h>
  #elif defined(HAVE_DDK_NTAPI_H)
  #include <ddk/ntapi.h>
  #elif defined(HAVE_NTAPI_H)
  #include <ntapi.h>
  #endif
  ]])
fi

AC_CHECK_HEADER(nvapi.h, [
  AC_DEFINE(HAVE_NVAPI_H,1,[Define to 1 if your compiler has the nvapi.h header file])
  ],[],[[
  #ifdef HAVE_WINDOWS_H
  #include <windows.h>
  #endif
  #ifdef HAVE_SAL_H
  #include "sal.h"
  #endif
  #ifndef __success
  #define __success(x)
  #endif
  #ifndef __in
  #define __in
  #endif
  #ifndef __out
  #define __out
  #endif
  #ifndef __in_ecount
  #define __in_ecount(x)
  #endif
  #ifndef __out_ecount
  #define __out_ecount(x)
  #endif
  #ifndef __in_opt
  #define __in_opt
  #endif
  #ifndef __out_opt
  #define __out_opt
  #endif
  #ifndef __inout
  #define __inout
  #endif
  #ifndef __inout_opt
  #define __inout_opt
  #endif
  #ifndef __inout_ecount
  #define __inout_ecount(x)
  #endif
  #ifndef __inout_ecount_full
  #define __inout_ecount_full(x)
  #endif
  #ifndef __inout_ecount_part_opt
  #define __inout_ecount_part_opt(x,y)
  #endif
  #ifndef __inout_ecount_full_opt
  #define __inout_ecount_full_opt(x,y)
  #endif
  #ifndef __out_ecount_full_opt
  #define __out_ecount_full_opt(x)
  #endif
]])


AC_CHECK_TYPES([socklen_t],[],[],[[
#if HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_SYS_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifdef HAVE_WINHTTP_H
#include <winhttp.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
]])

AC_CHECK_HEADER(net/if.h, [], [], [[
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
]])

AC_CHECK_HEADER(net/if_arp.h, [], [], [[
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
]])

AC_CHECK_HEADERS(sys/mount.h sys/swap.h sys/sensors.h, [], [], [[
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
]])

dnl check for broken Linux sysctl.h that causes deprecation warnings
saved_sysctl_h_CFLAGS=$CFLAGS
test "x${GCC}" != xyes || CFLAGS="$CFLAGS -Werror"
AC_CHECK_HEADERS([sys/sysctl.h], [], [], [[
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
]])
CFLAGS=$saved_sysctl_h_CFLAGS

AC_CHECK_HEADER(resolv.h, [], [], [[
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
]])

AC_CHECK_HEADERS(netinet/if_ether.h,[],[],[[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
]])

AC_CHECK_TYPES([struct lifconf, struct lifreq, struct ifconf, struct ifreq, struct ether_addr],[],[],[[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#ifdef HAVE_NETINET_ETHER_H
#include <netinet/ether.h>
#endif
]])

dnl Unfortunately on some 32 bit systems there is a problem with wx-widgets
dnl configuring itself for largefile support.  On these systems largefile
dnl support breaks C++ compiles by defining away many of the C standard library
dnl routines that should reside in namespace std::.  In order to get around
dnl problem first we have to check the largefile support macros.  Later we will
dnl use the macro SAH_LARGEFILE_BREAKS_CXX to check for the breakage.  If
dnl if breakage is found LARGEFILE_BREAKS_CXX is defined in config.h.  This
dnl define is checked in std_fixes.h and the appropriate functions are defined
dnl in order to solve the problem.
AC_SYS_LARGEFILE

AC_LANG_PUSH(C++)
SAH_LARGEFILE_BREAKS_CXX
SAH_HEADER_STDCXX
SAH_CHECK_NAMESPACES
AH_TEMPLATE([HAVE_STD_MIN],[Define to 1 if min is in namespace std::])
AH_TEMPLATE([HAVE_STD_MAX],[Define to 1 if max is in namespace std::])
AH_TEMPLATE([HAVE_STD_LOCALE],[Define to 1 if locale is in namespace std::])
AH_TEMPLATE([HAVE_STD_TRANSFORM],[Define to 1 if transform is in namespace std:: ])
SAH_FUNCS_IN_NAMESPACE([['min(0,0)'] ['max(0,0)'] ['transform((char *)0,(char *) 0,(char *)0,(int(*)(int))malloc)'] ['locale("")']],std)
AC_LANG_POP

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ether_ntoa setpriority sched_setscheduler strlcpy strlcat strcasestr strcasecmp sigaction getutent setutent getisax strdup _strdup strdupa _strdupa daemon stat64 putenv setenv unsetenv res_init strtoull localtime localtime_r gmtime gmtime_r uselocale _configthreadlocale ftok])

AC_CHECK_DECLS([_fpreset, fpreset],
    [],[],[[
#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
]])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

dnl utmp file location
for f in /etc/utmp /var/adm/utmp /var/run/utmp; do
    if test -f $f; then
        utmp_file=$f
    fi
done
if test -n "$utmp_file"; then
    AC_DEFINE_UNQUOTED(UTMP_LOCATION, "$utmp_file", utmp file location)
fi

dnl Check for /proc/self/psinfo (Solaris)
if test -e "/proc/self/psinfo"; then
    AC_DEFINE(HAVE__PROC_SELF_PSINFO, 1, [Define to 1 if /proc/self/psinfo exists])
fi

dnl Check for /proc/self/stat (Linux)
if test -e "/proc/self/stat"; then
    AC_DEFINE(HAVE__PROC_SELF_STAT, 1, [Define to 1 if /proc/self/stat exists])
fi

dnl Check for /proc/meminfo (Linux)
if test -e "/proc/meminfo"; then
    AC_DEFINE(HAVE__PROC_MEMINFO, 1, [Define to 1 if /proc/meminfo exists])
fi

dnl ---------- wxWidgets --------------------------------------------------
if test "${enable_manager}" = yes ; then
  BOINC_OPTIONS_WXWIDGETS([3.1.3])
else
  AM_CONDITIONAL([GUI_GTK], false)
fi

dnl ---------- libNotify --------------------------------------------------
if test "${enable_manager}" = yes ; then
  PKG_CHECK_MODULES(LIBNOTIFY, [libnotify])
fi

dnl ---------- XCB --------------------------------------------------------
if test "${enable_manager}" = yes; then
    PKG_CHECK_MODULES(XCB,xcb,have_Xcb="yes",[echo "WARNING: Xcb not found, x11_screensaver will not be built"])
    PKG_CHECK_MODULES(XCBATOM,xcb-atom,have_Xcbatom="yes",[echo "WARNING: Xcb-atom not found, x11_screensaver will not be built"])
fi
AM_CONDITIONAL(BUILD_X11_SCREENSAVER, [ test "$have_Xcb" = yes -a "$have_Xcbatom" = yes ])
dnl -----------------------------------------------------------------------

if test "${enable_boinczip}" = yes; then
dnl ---------- libzip (required for libboinc_zip) ----------------------------
PKG_CHECK_MODULES([LIBZIP], [libzip], [have_libzip=yes], [have_libzip=no])
AS_IF([test "x${have_libzip}" != "xyes"], [
  AC_MSG_ERROR([
================================================================================
ERROR: libzip development files not found. They are required for the BOINC library.

  Install libzip (e.g., libzip-dev on Debian/Ubuntu, libzip-devel on Fedora,
  or via vcpkg/homebrew on Windows/macOS) and re-run configure.
================================================================================
  ])
])
AC_SUBST([LIBZIP_CFLAGS])
AC_SUBST([LIBZIP_LIBS])
dnl -----------------------------------------------------------------------
fi

dnl Heres another way to set up host-specific stuff
AM_CONDITIONAL(OS_DARWIN, [echo $host_os | grep '^darwin' > /dev/null])
if echo $host_os | grep '^darwin' >/dev/null ; then
  if test `uname -r | sed 's/\.//g'` -lt 800 ; then
    AC_DEFINE_UNQUOTED(DARWIN_10_3, [1],[Define to 1 if compiling under OS X 10.3 or earlier])
  fi
fi

AM_CONDITIONAL(OS_LINUX, [echo $host_os | grep '^linux' > /dev/null])
AM_CONDITIONAL(OS_FREEBSD, [echo $host_os | grep '^freebsd' > /dev/null])
dnl In case anyone wants to try building the windows code using mingw!
AM_CONDITIONAL(OS_WIN32, [echo $host_os | grep -E '^mingw|^winnt' > /dev/null])
AM_CONDITIONAL(OS_WIN32_MINGW, [echo $host_os | grep '^mingw' > /dev/null])
dnl or OS2
AM_CONDITIONAL(OS_OS2, [echo $host_os | grep '^os2' > /dev/null])
AM_CONDITIONAL(OS_ARM_LINUX, [echo $host_alias | grep '^arm-linux' > /dev/null])
AM_CONDITIONAL(OS_ARMV6_LINUX, [echo $host_alias | grep '^armv6-linux' > /dev/null])
AM_CONDITIONAL(ANDROID, [test x"${ANDROID}" = xyes])

AM_CONDITIONAL(BUILD_APPS_WITH_VCPKG, [test "${enable_apps_vcpkg}" = yes])
AM_CONDITIONAL(BUILD_WITH_VBOX, [test "${enable_apps_vbox}" = yes])
AM_CONDITIONAL(BUILD_WITH_MINGW, [test "${enable_apps_mingw}" = yes])
AM_CONDITIONAL(BUILD_WITH_GUI, [test "${enable_apps_gui}" = yes])

dnl Whether to build fcgi components
AM_CONDITIONAL(ENABLE_FCGI,[test "${enable_fcgi}" = yes])

dnl tell automake about whether to build client and/or server
AM_CONDITIONAL(DYNAMIC_CLIENT, [test "${dynamic_client}" = yes])
AM_CONDITIONAL(ENABLE_SERVER, [test "${enable_server}" = yes])
AM_CONDITIONAL(ENABLE_CLIENT, [test "${enable_client}" = yes])
AM_CONDITIONAL(ENABLE_MANAGER, [ test "x${ac_cv_have_wxwidgets}" = xyes -a "${enable_manager}" = yes ])
AM_CONDITIONAL(ENABLE_LIBRARIES, [test "${enable_libraries}" = yes])
AM_CONDITIONAL(ENABLE_BOINCZIP, [test "${enable_boinczip}" = yes])
AM_CONDITIONAL(ENABLE_APPS, [test "${enable_apps}" = yes])
AM_CONDITIONAL(ENABLE_UNIT_TESTS, [test "${enable_unit_tests}" = yes])
AM_CONDITIONAL(ENABLE_BOINCCRYPT, [test "x${enable_server}" = xyes  || test "x${enable_client}" = xyes ])
AM_CONDITIONAL(INSTALL_HEADERS, [test "${enable_install_headers}" = yes])
AM_CONDITIONAL(HAVE_CUDA_LIB, [test "${enable_client}" = yes -a -f ./coprocs/CUDA/posix/${boinc_platform}/libcudart.so])
AM_CONDITIONAL(BUILD_WITH_OPENCL, [test "x$with_opencl" = "xyes"])

dnl ======================================================================
dnl some more vodoo required for building portable client-binary (client, clientgui)
dnl ======================================================================

CLIENTLIBS=

SAH_CHECK_LIB([m],[sin], [
    AC_DEFINE([HAVE_LIBM],[1],[Define to 1 if you have the math library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
if test "$enable_release_client" = yes -a "$disable_static_linkage" = no ; then
    SAH_CHECK_LIB([c],[atexit], [
        AC_DEFINE([HAVE_LIBC],[1],[Define to 1 if you have the c library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
       echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
fi
    SAH_CHECK_LIB([pthread],[pthread_join],[
        AC_DEFINE([HAVE_LIBPTHREAD],[1],[Define to 1 if you have the pthread library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
       echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
if test "$enable_release_client" = yes -a "$disable_static_linkage" = no ; then
if test x${ac_cv_cxx_compiler_gnu} != xno ; then
  SAH_CHECK_LIB([gcc_eh],[_Unwind_Resume],[
    AC_DEFINE([HAVE_LIBGCC_EH],[1],[Define to 1 if you have the gcc_eh library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([gcc],[sscanf], [
    AC_DEFINE([HAVE_LIBGCC],[1],[Define to 1 if you have the gcc library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([c_nonshared],[fopen],[
    AC_DEFINE([HAVE_LIBC_NONSHARED],[1],[Define to 1 if you have the c_nonshared library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([stdc++],[sscanf],[
    AC_DEFINE([HAVE_LIBSTDC__],[1],[Define to 1 if you have the stdc++ library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
fi
fi
SAH_CHECK_LIB([nvapi],[fopen],[
    AC_DEFINE([HAVE_LIBNVAPI],[1],[Define to 1 if you have the NVIDIA API library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
if test ${isWIN32} = "yes" ; then
  SAH_CHECK_LIB([gdi32],[fopen],[
      AC_DEFINE([HAVE_LIBWGDI32],[1],[Define to 1 if you have the gdi32 library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
     found_msvcrt=no
  SAH_CHECK_LIB([msvcr110],[fopen],[
      found_msvcrt=yes
      AC_DEFINE([HAVE_LIBMSVCR110],[1],[Define to 1 if you have the msvcr110 library])
      AC_DEFINE([HAVE_MSVCRT],[1],[Define to 1 if you have a visual c runtime library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  if test $found_msvcrt = no ; then
    SAH_CHECK_LIB([msvcr100],[fopen],[
        found_msvcrt=yes
        AC_DEFINE([HAVE_LIBMSVCR100],[1],[Define to 1 if you have the msvcr100 library])
        AC_DEFINE([HAVE_MSVCRT],[1],[Define to 1 if you have a visual c runtime library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
    echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  fi
  if test $found_msvcrt = no ; then
    SAH_CHECK_LIB([msvcr90d],[fopen],[
        found_msvcrt=yes
        AC_DEFINE([HAVE_LIBMSVCR90D],[1],[Define to 1 if you have the msvcr90d library])
        AC_DEFINE([HAVE_MSVCRT],[1],[Define to 1 if you have a visual c runtime library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
    echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  fi
  if test $found_msvcrt = no ; then
    SAH_CHECK_LIB([msvcr90],[fopen],[
        found_msvcrt=yes
        AC_DEFINE([HAVE_LIBMSVCR90],[1],[Define to 1 if you have the msvcr90 library])
        AC_DEFINE([HAVE_MSVCRT],[1],[Define to 1 if you have a visual c runtime library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
    echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  fi
  SAH_CHECK_LIB([wininet],[fopen],[
      AC_DEFINE([HAVE_LIBWININET],[1],[Define to 1 if you have the wininet library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([wsock32],[fopen],[
      AC_DEFINE([HAVE_LIBWSOCK32],[1],[Define to 1 if you have the wsock32 library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([userenv],[fopen],[
      AC_DEFINE([HAVE_LIBUSERENV],[1],[Define to 1 if you have the userenv library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([iphlpapi],[fopen],[
      AC_DEFINE([HAVE_LIBIPHLPAPI],[1],[Define to 1 if you have the iphlpapi library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([sensapi],[fopen],[
      AC_DEFINE([HAVE_LIBSENSAPI],[1],[Define to 1 if you have the sensapi library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([wtsapi32],[fopen],[
      AC_DEFINE([HAVE_LIBWTSAPI32],[1],[Define to 1 if you have the wtsapi32 library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([winhttp],[fopen],[
      AC_DEFINE([HAVE_LIBWINHTTP],[1],[Define to 1 if you have the WinHttp library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
  SAH_CHECK_LIB([secur32],[fopen],[
      AC_DEFINE([HAVE_LIBSECUR32],[1],[Define to 1 if you have the secur32 library])
      CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
     echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
fi

svlibs=$LIBS
if test "${ac_cv_func_res_init}" != "yes" ; then
  AC_CHECK_LIB([resolv],[res_init],[
    ac_cv_func_res_init="yes"
    AC_CHECK_FUNCS([res_init])
  ])
fi
LIBS=$svlibs

if test "$enable_xss" = yes -a "$have_Xss" = yes; then
    SAH_CHECK_LIB([Xss],[XScreenSaverAllocInfo],[
        AC_DEFINE([HAVE_XSS],[1],[Define to 1 if you have xss library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
    SAH_CHECK_LIB([X11],[XOpenDisplay],[
        AC_DEFINE([HAVE_X11],[1],[Define to 1 if you have X11 library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
    echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
fi

SAH_CHECK_LIB([resolv],[res_query],[
    AC_DEFINE([HAVE_RESOLV],[1],[Define to 1 if you have the resolv library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5

if test "${disable_static_linkage}" != yes -a "${enable_client_release}" = yes; then

   echo "----------"
   echo "NOTE: Building portable client binaries"
   echo "----------"

if test x$ac_cv_cxx_compiler_gnu != xno ; then
SAH_CHECK_LDFLAG([-nodefaultlibs],[CLIENTLIBS="-nodefaultlibs ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5

SAH_CHECK_LDFLAG([-lSystemStubs],[LIBS="${LIBS} -lSystemStubs"])
   echo DEBUG: LIBS=${LIBS} >&5
fi

fi

AM_CONDITIONAL([ENABLE_CLIENT_RELEASE],[ test "${disable_static_linkage}" != yes -a "${enable_client_release}" = yes ])
AM_CONDITIONAL([BUILD_STATIC_LIBS],[test "x${enable_static}" != xno])

AC_SUBST(CLIENTLIBS)

## --------------------
## some more tweaking to turn non-standard libs into statically linked ones

if test "${enable_debug}" = "yes" ; then
    CLIENTGUIFLAGS="${CLIENTGUIFLAGS} -D_DEBUG -DDEBUG"
else
    # disable wxWidgets debug support which is by default enabled since 2.9.1
    CLIENTGUIFLAGS="${CLIENTGUIFLAGS} -DNDEBUG"
fi

CLIENTGUILIBS="${WX_LIBS}"

if test "${enable_client_release}" = "yes" ; then
   if test "x${WX_LIBS_STATIC}" = "x" ; then
     WX_LIBS_STATIC="${WX_LIBS}"
   fi
   SAH_STATICIZE_LDFLAGS([${WX_LIBS_STATIC} ${GTK_LIBS}],[CLIENTGUILIBS])
   SAH_STATICIZE_LDFLAGS([${GLUT_LIBS}],[GLUT_LIBS])
   SAH_STATICIZE_LDFLAGS([${BOINC_EXTRA_LIBS}],[BOINC_EXTRA_LIBS])



   ## NOTE: its *extremely* important to always compile with __NO_CTYPE
   ## defined, because this avoids GLIBC_2.3 symbols being pulled in via ctype.h-macros,
   CPPFLAGS="${CPPFLAGS} -D__NO_CTYPE"
fi

AC_SUBST(BOINC_EXTRA_LIBS)
AC_SUBST(CLIENTGUIFLAGS)
AC_SUBST(CLIENTGUILIBS)
dnl ======================================================================
dnl Oh! the joys of standards!
AC_PATH_PROG(WHOAMI,[whoami logname],"not found")
if test "x${WHOAMI}" = "xnot found" ; then
  who am i 2>/dev/null && WHOAMI="who am i | awk '{print $1}'"
fi
if test "x${WHOAMI}" = "xnot found"  -a -x "/usr/ucb/ps" ; then
  WHOAMI="/usr/ucb/ps u $$ | tail -1 | awk '{print $1}'"
fi
if test  "x${WHOAMI}" = "xnot found" ; then
  WHOAMI="ps u $$ | tail -1 | awk '{print $1}'"
fi


dnl Some info about the person compiling used for making some packages.
LOGNAME=`${WHOAMI}`
FULLNAME=`grep ^${LOGNAME}: /etc/passwd | head -n 1 | awk -F: '{print $5}'`
if test -z "${FULLNAME}" ; then
  FULLNAME=`ypcat passwd 2>/dev/null | grep ^${LOGNAME}: | head -n 1 | awk -F: '{print $5}'`
fi
if test -z "${FULLNAME}" ; then
  FULLNAME=`nidump passwd / 2>/dev/null | grep ^${LOGNAME}: | head -n 1 | awk -F: '{print $5}'`
fi
if test -z "${FULLNAME}" ; then
  FULLNAME=$LOGNAME
fi
AC_SUBST(LOGNAME)
AC_SUBST(FULLNAME)

AC_CONFIG_FILES([
                 version.h
                 api/Makefile
                 api/libboinc_api.pc
                 api/libboinc_graphics2.pc
                 api/libboinc_opencl.pc
                 apps/Makefile
                 clientgui/Makefile
                 clientgui/res/Makefile
                 clientgui/skins/Makefile
                 clientscr/Makefile
                 client/Makefile
                 client/win/boinc_path_config.py:py/boinc_path_config.py.in
                 client/scripts/Makefile
                 client/scripts/boinc-client
                 client/scripts/boinc-client.service
                 db/Makefile
                 doc/Makefile
                 doc/manpages/Makefile
                 html/Makefile
                 lib/Makefile
                 lib/libboinc.pc
                 lib/libboinc_crypt.pc
                 lib/libboinc_fcgi.pc
                 locale/Makefile
                 Makefile
                 py/Boinc/version.py
                 py/Makefile
                 py/boinc_path_config.py:py/boinc_path_config.py.in
                 py/setup.py
                 sched/boinc_path_config.py:py/boinc_path_config.py.in
                 sched/Makefile
                 packages/generic/sea/Makefile
                 packages/solaris/CSW/Makefile
                 packages/solaris/CSW/boincclient/Makefile
                 packages/solaris/CSW/boincclient/pkginfo
                 packages/solaris/CSW/boincclient/prototype
                 packages/solaris/CSW/boincdevel/Makefile
                 packages/solaris/CSW/boincdevel/pkginfo
                 packages/solaris/CSW/boincdevel/prototype
                 packages/solaris/CSW/boinclibs/Makefile
                 packages/solaris/CSW/boinclibs/pkginfo
                 packages/solaris/CSW/boinclibs/prototype
                 packages/solaris/CSW/boincmanager/Makefile
                 packages/solaris/CSW/boincmanager/pkginfo
                 packages/solaris/CSW/boincmanager/prototype
                 samples/Makefile
                 tools/boinc_path_config.py:py/boinc_path_config.py.in
                 tools/Makefile
                 vda/Makefile
                 zip/Makefile
                 zip/libboinc_zip.pc
                 m4/Makefile
		])


dnl AC_CONFIG_HEADER([config.h])
AC_CONFIG_HEADERS([config.h])

AC_OUTPUT

if test "$enable_libraries" = "yes" -a "$have_glut" = yes -a "$have_jpeg" = 1; then
  configured_to_build="${configured_to_build} libraries_graphics"
fi

if test "${enable_manager}" = yes -a "$have_Xcb" = yes -a "$have_Xcbatom" = yes; then
  configured_to_build="${configured_to_build} manager_clientscr"
fi

if test "$enable_apps" = yes -a "$enable_apps_gui" = yes; then
  configured_to_build="${configured_to_build} apps_gui"
fi

if test "$enable_apps" = yes -a "$enable_apps_vcpkg" = yes; then
  configured_to_build="${configured_to_build} apps_vcpkg"
fi

if test "$enable_apps" = yes -a "$enable_apps_vbox" = yes; then
  configured_to_build="${configured_to_build} apps_vbox"
fi

if test "$enable_apps" = yes -a "$enable_apps_mingw" = yes; then
  configured_to_build="${configured_to_build} apps_mingw"
fi

echo "--- Configuring BOINC AC_PACKAGE_VERSION (${build_state}) ---"
echo "--- Build Components: (${configured_to_build}) ---"
