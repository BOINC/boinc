dnl -*- autoconf -*-

dnl $Id$

dnl not sure exactly what the minimum version is (but 2.13 wont work)
AC_PREREQ(2.58)

dnl Set the BOINC version here.  You can also use the set-version script.
AC_INIT(BOINC, 7.1.0)
AC_CONFIG_MACRO_DIR([m4])
LIBBOINC_VERSION=`echo ${PACKAGE_VERSION} | sed 's/\./:/g'`
AC_SUBST([LIBBOINC_VERSION])

AC_CANONICAL_TARGET

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)
AM_INIT_AUTOMAKE(dist-zip)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR(lib/shmem.cpp)

AC_REVISION([$Revision$]) 
REV=`echo '$Revision$' | awk "{print $2}"`
RDATE=`date '+%Y.%m.%d'`
if test -d .svn ; then
  REV=`svn info | grep Revision | awk '{print $2}'`
  RDATE=`svn info | grep "Last Changed Date:" | awk '{print $4}' | sed 's/-/./g'`
fi
AC_SUBST(REV)
AC_SUBST(RDATE)

dnl turn dependency tracking on by default
if test x${enable_dependency_tracking} != xno ; then
  enable_dependency_tracking=yes
fi

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
dnl ------
dnl Workaround for autoconf >= 2.65 backwards incompatibility
m4_pattern_allow([AC_PROG_OBJCXX])
m4_ifdef([AC_PROG_OBJCXX],[AC_PROG_OBJCXX],)
dnl ------
AC_PROG_F77
AC_PROG_CPP
AC_PROG_MAKE_SET
SAH_LINKS
AC_LANG_PUSH(C)
AM_PROG_CC_C_O

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

AC_ARG_ENABLE(pkg-libs, 
    AS_HELP_STRING([--enable-pkg-libs],
                   [Builds and installs components that would be present in a
		   boinc-libs package.  This is a synonym for --disable-server
		   --disable-client --enable-libraries --disable-static
		   --disable-install-headers --disable-manager]),
    [
     enable_libraries=yes
     enable_server=no
     enable_client=no
     enable_manager=no
     enable_install_headers=no
     enable_static=no
    ],
    [])

AC_ARG_ENABLE(pkg-devel, 
    AS_HELP_STRING([--enable-pkg-devel],
                   [Builds and installs components that would be present in a
		   boinc-devel package.  This is a synonym for --disable-server
		   --disable-client --enable-libraries --enable-install-headers
		   --disable-manger]),
    [
     enable_libraries=yes
     enable_server=no
     enable_client=no
     enable_manager=no
     enable_install_headers=yes
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
    ],
    [])

BOINC_SET_COMPILE_FLAGS

configured_to_build=

if test x$enable_client = xyes ; then
  configured_to_build="client"
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
Copyright (C) 2005 University of California

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
_AM_DEPENDENCIES([OBJC])
OBJC="$CC"
AC_SUBST(OBJC)
OBJCFLAGS="-ObjC"
AC_SUBST(OBJCFLAGS)

dnl LT_INIT
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_SUBST(PICFLAGS,${lt_prog_compiler_pic})

SAH_OPTION_BITNESS

dnl Determine the BOINC platform given the target arch-platform-os.
BOINC_PLATFORM

if ( test "${enable_client_release}" = yes ) && ( test "${enable_client}" != yes ); then
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
      AC_MSG_WARN([ mysql not found.
======================================================================
WARNING: trying to build BOINC-server, but MYSQL was not be found.
If you dont want to build the server you should use --disable-server.

I am continuing now as if --disable-server had been specified.

======================================================================
])
   enable_server=no
   fi
fi



dnl Checks for libraries.

dnl NOTE: Libraries subject to version changes, or those that might not be
dnl installed everywhere should be linked statically when building portable 
dnl binaries.  Put libraries that should be linked statically on every platform
dnl in the STATIC_LIB_LIST variable before calling SAH_CHECK_LIB.  (Note: Only 
dnl the '*' wildcard is supported).
STATIC_LIB_LIST="stdc++* gcc gcc_s gcc_eh atk* gtk* gdk* gmodule* gthread* gobject* glib* pango* cairo* wx* tiff* jpeg* png* Xi glut socket z expat* krb5support freetype fontconfig* resolv* idn c_nonshared gpg* gss* gnutls sasl2 sqlite3"

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

dnl ---------- libcurl (m4/libcurl.m4) ------------------------------

if test "${enable_client}" = yes; then
   LIBCURL_CHECK_CONFIG([yes], [7.17.1], [haveCurl=yes], [haveCurl=no])
   
   if test "${haveCurl}" != yes; then
      AC_MSG_ERROR([
================================================================================
ERROR: could not find (recent enough) development-libs for libcurl.

  This library is required to build the boinc-client.
  (If you don't want to build the client, use --disable-client with configure.

  If libcurl-dev is installed on your system, make sure that the script 
  'curl-config' is found in your PATH, and that 
  'curl-config --version' gives something recent enough (see above).

  You can download libcurl from: http://curl.haxx.se/

================================================================================
	])
   else
      ## add libcurl et al. to the list of statically linked libs
      STATIC_LIB_LIST="${STATIC_LIB_LIST} curl idn ssh2 crypto ssl krb5 k5crypto gssapi_krb5 com_err resolv lber ldap socket nsl z rt gcrypt gpg-error"
      CPPFLAGS="${CPPFLAGS} ${LIBCURL_CPPFLAGS}"
      CURL_LIB_PATHS=`echo $LIBCURL | sed 's/[^[a-zA-Z]]*-l[^ ]*//g'`

      if test "${enable_debug}" = yes; then
         echo "LIBCURL = ${LIBCURL}"
	 echo "LIBCURL_CPPFLAGS = ${LIBCURL_CPPFLAGS}"
	 echo "CURL_LIB_PATHS = ${CURL_LIB_PATHS}"
	 echo "LDFLAGS = ${LDFLAGS}"
      fi
      BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${LIBCURL}"

   fi	

fi

dnl ---------- SSL (m4/check_ssl.m4)
CHECK_SSL
if test "${found_ssl}" = "yes"; then
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
#SAH_CHECK_LIB([wsock32], [fopen], 
#   [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
#SAH_CHECK_LIB([wsock], [fopen], 
#   [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([socket], [bind], 
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([z], [gzopen], 
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([cups], [md5_finish],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])

LIBS=${SAVELIBS_AAF}

dnl check for pthread
ACX_PTHREAD(AC_DEFINE(HAVE_PTHREAD,1, [Have pthread]))
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

dnl if --sysconfdir was not specified, default ${sysconfdir} to '/etc/'.
dnl this is done to preserve the behaviour of previous BOINC versions 
dnl which always installed to /etc/ in client/scripts/. Now we install
dnl to ${sysconfdir}/.
dnl the default value for ${sysconfdir} set by automake is '${prefix}/etc',
dnl but we also test for an empty string
if test "${sysconfdir}" = '${prefix}/etc' -o "x${sysconfdir}" = "x"; then
  sysconfdir=/etc
fi

AC_DEFINE_UNQUOTED(BOINC_PREFIX,"`eval echo ${prefix}`",[Directory tree where BOINC will be installed])
AC_DEFINE_UNQUOTED(BOINC_EXECPREFIX,"`eval echo ${bindir}`",[Directory where BOINC executables will be installed])
AC_DEFINE_UNQUOTED(BOINC_LIBDIR,"`eval echo ${libdir}`",[Directory where BOINC libraries will be installed])

dnl ---------- decide wether to build the graphical boinc-api libs or not
dnl check for X11 
AC_PATH_X
AC_PATH_XTRA

dnl check for GL library
AX_CHECK_GL
AX_CHECK_GLU

dnl check for glut and prerequesites to the glut-test: libXmu and libXi
AC_CHECK_LIB([Xmu], [fopen], [have_Xmu="yes"], [have_Xmu="no"])
AC_CHECK_LIB([Xi], [fopen], [have_Xi="yes"], [have_Xi="no"])

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

   AC_CHECK_HEADERS([gl.h glu.h glut.h glaux.h GL/gl.h GL/glu.h GL/glut.h GL/glaux.h OpenGL/gl.h OpenGL/glu.h OpenGL/glut.h OpenGL/glaux.h GLUT/glut.h MesaGL/gl.h MesaGL/glu.h MesaGL/glut.h MesaGL/glaux.h libnotify/notify.h gtk/gtk.h xlocale.h])

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

AM_CONDITIONAL(BUILD_GRAPHICS_API, [ test "$have_glut" = yes ])

dnl check for X screen saver lib
AC_CHECK_LIB([Xss], [XScreenSaverAllocInfo], [have_Xss="yes"], [have_Xss="no"])
AC_CHECK_HEADER([X11/extensions/scrnsaver.h], [have_Xss="yes"], [have_Xss="no"])
if test "$have_Xss" == no; then
    AC_MSG_WARN([libxss missing, disabling X ScreenSaver user idle detection])
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

#ifndef HAVE_XLOCALE_H
#define NO_PER_THREAD_LOCALE 1
#endif

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
AC_CHECK_HEADERS(windows.h sys/types.h sys/un.h arpa/inet.h dirent.h grp.h fcntl.h inttypes.h stdint.h malloc.h alloca.h memory.h netdb.h netinet/in.h netinet/tcp.h netinet/ether.h signal.h strings.h sys/auxv.h sys/file.h sys/fcntl.h sys/ipc.h sys/ioctl.h sys/msg.h sys/param.h sys/resource.h sys/select.h sys/sem.h sys/shm.h sys/sockio.h sys/socket.h sys/stat.h sys/statvfs.h sys/statfs.h sys/systeminfo.h sys/time.h sys/types.h sys/utsname.h sys/vmmeter.h sys/wait.h unistd.h utmp.h errno.h procfs.h ieeefp.h setjmp.h)

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

AC_CHECK_HEADERS(sys/sysctl.h sys/mount.h sys/swap.h, [], [], [[
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
]])

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
AC_FUNC_ALLOCA
if test "${ac_cv_func_alloca_works}" = "yes" ; then
  ac_cv_func_alloca="yes"
fi
AC_CHECK_FUNCS(alloca _alloca __builtin_alloca ether_ntoa setpriority sched_setscheduler strlcpy strlcat strcasestr strcasecmp sigaction getutent setutent getisax strdup strdupa daemon stat64 putenv setenv unsetenv res_init strtoull)

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
  BOINC_OPTIONS_WXWIDGETS([2.8.0])
fi

dnl ---------- libNotify --------------------------------------------------
if test "${enable_manager}" = yes ; then
  pkg_config_args=libnotify
  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

  LIBNOTIFY_CFLAGS=`$PKG_CONFIG libnotify --cflags`
  LIBNOTIFY_LIBS=`$PKG_CONFIG libnotify --libs`

  AC_SUBST(LIBNOTIFY_CFLAGS)
  AC_SUBST(LIBNOTIFY_LIBS)
fi

dnl ---------- Sqlite3 ----------------------------------------------------
if test "${enable_manager}" = yes ; then
  AM_PATH_SQLITE3
fi

dnl ---------- XCB --------------------------------------------------------
if test "${enable_manager}" = yes; then
    PKG_CHECK_MODULES(XCB,xcb,have_Xcb="yes",[echo "WARNING: Xcb not found, x11_screensaver will not be built"])
    PKG_CHECK_MODULES(XCBATOM,xcb-atom,have_Xcbatom="yes",[echo "WARNING: Xcb-atom not found, x11_screensaver will not be built"])
fi
AM_CONDITIONAL(BUILD_X11_SCREENSAVER, [ test "$have_Xcb" = yes -a "$have_Xcbatom" = yes ]) 
dnl -----------------------------------------------------------------------

dnl Heres another way to set up host-specific stuff
AM_CONDITIONAL(OS_DARWIN, [echo $host_os | grep '^darwin' > /dev/null])
if echo $host_os | grep '^darwin' >/dev/null ; then
  if test `uname -r | sed 's/\.//g'` -lt 800 ; then
    AC_DEFINE_UNQUOTED(DARWIN_10_3, [1],[Define to 1 if compiling under OS X 10.3 or earlier])
  fi
fi

AM_CONDITIONAL(OS_LINUX, [echo $host_os | grep '^linux' > /dev/null])
dnl In case anyone wants to try building the windows code using mingw!
AM_CONDITIONAL(OS_WIN32_MINGW, [echo $host_os | grep '^mingw' > /dev/null])
dnl or OS2
AM_CONDITIONAL(OS_OS2, [echo $host_os | grep '^os2' > /dev/null])

dnl Whether to build fcgi components
AM_CONDITIONAL(ENABLE_FCGI,[test "${enable_fcgi}" = yes])

dnl tell automake about whether to build client and/or server
AM_CONDITIONAL(DYNAMIC_CLIENT, [test "${dynamic_client}" = yes])
AM_CONDITIONAL(ENABLE_SERVER, [test "${enable_server}" = yes])
AM_CONDITIONAL(ENABLE_CLIENT, [test "${enable_client}" = yes])
AM_CONDITIONAL(ENABLE_MANAGER, [ test "x${ac_cv_have_wxwidgets}" = xyes -a "${enable_manager}" = yes ])
AM_CONDITIONAL(ENABLE_LIBRARIES, [test "${enable_libraries}" = yes])
AM_CONDITIONAL(INSTALL_HEADERS, [test "${enable_install_headers}" = yes])
AM_CONDITIONAL(HAVE_CUDA_LIB, [test "${enable_client}" = yes -a -f ./coprocs/CUDA/posix/${boinc_platform}/libcudart.so])

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
SAH_CHECK_LIB([wsock32],[fopen],[
    AC_DEFINE([HAVE_LIBWSOCK32],[1],[Define to 1 if you have the wsock32 library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5

svlibs=$LIBS
if test "${ac_cv_func_res_init}" != "yes" ; then 
  AC_CHECK_LIB([resolv],[res_init],[
    ac_cv_func_res_init="yes"
    AC_CHECK_FUNCS([res_init])
  ])
fi
LIBS=$svlibs

if test "$have_Xss" == yes; then
    SAH_CHECK_LIB([Xss],[XScreenSaverAllocInfo],[
        AC_DEFINE([HAVE_XSS],[1],[Define to 1 if you have xss library])
        CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
    echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
fi

SAH_CHECK_LIB([resolv],[res_query],[
    AC_DEFINE([HAVE_RESOLV],[1],[Define to 1 if you have the resolv library])
    CLIENTLIBS="${sah_lib_last} ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5

if ( test "${disable_static_linkage}" != yes ) && ( test "${enable_client_release}" = yes ); then

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

AM_CONDITIONAL([ENABLE_CLIENT_RELEASE],[ ( test "${disable_static_linkage}" != yes ) && ( test "${enable_client_release}" = yes ) ])
AM_CONDITIONAL([BUILD_STATIC_LIBS],[test "x${enable_static}" != xno])

AC_SUBST(CLIENTLIBS)

## --------------------
## some more tweaking to turn non-standard libs into statically linked ones

if test "${enable_debug}" = yes; then
    CLIENTGUIFLAGS="${CLIENTGUIFLAGS} -D_DEBUG -DDEBUG"
fi

AC_ARG_ENABLE(wx-debug,
  AS_HELP_STRING([--enable-wx-debug],
                 [ use wxWidgets debug libraries ]),
  [enable_wx_debug="yes"
   AC_DEFINE(USE_WX_DEBUG,[1],[Define to 1 to use the wxWidgets debug libraries])
   ],
  [enable_wx_debug="no"])

CLIENTGUILIBS="${WX_LIBS} ${SQLITE3_LIBS} ${GTK_LIBS}"

if test "${enable_client_release}" = "yes" ; then
   if test "x${WX_LIBS_STATIC}" = "x" ; then
     WX_LIBS_STATIC="${WX_LIBS}"
   fi
   SAH_STATICIZE_LDFLAGS([${WX_LIBS_STATIC} ${SQLITE3_LIBS} ${GTK_LIBS}],[CLIENTGUILIBS])
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
                 apps/Makefile
                 clientgui/Makefile
                 clientgui/res/Makefile
                 clientscr/Makefile
                 client/Makefile
                 client/win/boinc_path_config.py:py/boinc_path_config.py.in
                 client/scripts/Makefile
                 client/scripts/boinc-client
                 db/Makefile
                 doc/Makefile
                 doc/manpages/Makefile
                 lib/Makefile
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
                 test/boinc_path_config.py:py/boinc_path_config.py.in
                 test/Makefile
                 test/version.inc
                 tools/boinc_path_config.py:py/boinc_path_config.py.in
                 tools/Makefile
                 vda/Makefile
                 zip/Makefile
                 zip/zip/Makefile
                 zip/unzip/Makefile
                 m4/Makefile
		])


dnl AC_CONFIG_HEADER([config.h])
AC_CONFIG_HEADERS([config.h])

AC_OUTPUT

echo "--- Configuring BOINC AC_PACKAGE_VERSION (${build_state}) ---"
echo "--- Build Components: (${configured_to_build}) ---"
