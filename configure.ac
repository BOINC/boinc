dnl -*- autoconf -*-

dnl $Id$

dnl NOTE: on milkyway.ssl and jill.ssl , need autoconf/automake et al from quarls
dnl directory; the _autosetup script does this.

dnl not sure exactly what the minimum version is (but 2.13 wont work)
AC_PREREQ(2.57)

dnl Set the BOINC version here.  You can also use the set-version script.
AC_INIT(BOINC, 6.4.7)
LIBBOINC_VERSION=`echo ${PACKAGE_VERSION} | sed 's/\./:/g'`
AC_SUBST([LIBBOINC_VERSION])


AC_ARG_ENABLE(debug, 
    AS_HELP_STRING([--enable-debug],
                   [enable tracing and debugging flags for all components]),
    [enable_debug="$enableval"],
    [])

AC_ARG_ENABLE(unicode, 
    AS_HELP_STRING([--enable-unicode],
                   [enable building the manager with unicode support]),
    [enable_unicode="$enableval"],
    [])

AC_ARG_ENABLE(server, 
    AS_HELP_STRING([--disable-server],
                   [disable building the scheduling server]),
    [],
    [enable_server=yes])

AC_ARG_ENABLE(client, 
    AS_HELP_STRING([--disable-client],
                   [disable building the client]),
    [],
    [enable_client=yes])

m4_divert_once([HELP_ENABLE],
  AS_HELP_STRING([], [Default: --enable-server --enable-client: 
                      builds both server and client]))

if test "${enable_server}" = yes ; then
    if test "${enable_client}" = yes ; then
        configured_to_build='server & client'
    else
        configured_to_build='server only'
    fi
else
    if test "${enable_client}" = yes ; then
        configured_to_build='client only'
    else
        configured_to_build='libraries only'
    fi
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
    [enable_client_release=yes;disable_static_linkage=no],
    [enable_client_release=no;disable_static_linkage=yes])


if test "${enable_debug}" = yes ; then
    build_state='Debug'
else
    build_state='Release'
fi

echo "--- Configuring BOINC AC_PACKAGE_VERSION (${build_state}) ---"
echo "--- Build Components: (${configured_to_build}) ---"

AC_CANONICAL_TARGET

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)
AM_INIT_AUTOMAKE(dist-zip)

AC_CONFIG_SRCDIR(lib/shmem.cpp)

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

AC_REVISION([$Revision$])

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
AC_SUBST([TOP_SOURCE_DIR], [`(cd \`dirname "$0"\` && pwd)`])

## commented this out to turn all maintainer-mode features ON by default,
## such as automatic reconfiguring on changes in Makefile.am's and configure.ac
## AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_RANLIB
SAH_LINKS
AM_PROG_CC_C_O

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

dnl enable debug mode on all components using standard debug flags
if test "${enable_debug}" = yes; then
    CXXFLAGS="$CXXFLAGS -DDEBUG -D_DEBUG"
    CFLAGS="$CFLAGS -DDEBUG -D_DEBUG"

    AC_SUBST(CXXFLAGS)
    AC_SUBST(CFLAGS)
fi

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
STATIC_LIB_LIST="stdc++* gcc gcc_s gcc_eh atk* gtk* gdk* gmodule* gthread* gobject* 
glib* pango* cairo* wx* tiff* jpeg* png* Xi glut socket z expat* krb5support
freetype fontconfig* resolv* idn"

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

dnl m4/boinc_getsockopt.m4
BOINC_GETSOCKOPT_TYPE

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
      STATIC_LIB_LIST="${STATIC_LIB_LIST} curl idn crypto ssl krb5 k5crypto gssapi_krb5 com_err resolv"
      CPPFLAGS="${CPPFLAGS} ${LIBCURL_CPPFLAGS}"
      CURL_LIB_PATHS=`echo $LIBCURL | sed 's/[^[a-zA-Z]]*-l[^ ]*//g'`
      LDFLAGS="$CURL_LIB_PATHS $LDFLAGS"

      if test "${enable_debug}" = yes; then
         echo "LIBCURL = ${LIBCURL}"
	 echo "LIBCURL_CPPFLAGS = ${LIBCURL_CPPFLAGS}"
	 echo "CURL_LIB_PATHS = ${CURL_LIB_PATHS}"
	 echo "LDFLAGS = ${LDFLAGS}"
      fi

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
#    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
#SAH_CHECK_LIB([wsock], [fopen], 
#    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([socket], [bind], 
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([z], [gzopen], 
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([cups], [md5_finish],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${sah_lib_last}"])
SAH_CHECK_LIB([curl], [curl_easy_init],
    [BOINC_EXTRA_LIBS="${BOINC_EXTRA_LIBS} ${LIBCURL}"],, [${LIBCURL}])

LIBS=${SAVELIBS_AAF}

dnl check for pthread
ACX_PTHREAD(AC_DEFINE(HAVE_PTHREAD,1, [Have pthread]))
CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"


dnl ---------- decide wether to build the graphical boinc-api libs or not
dnl check for X11 
AC_PATH_X
AC_PATH_XTRA

dnl check for GL library
AX_CHECK_GL
AX_CHECK_GLU

dnl check for glut and prerequesites to the glut-test: libXmu and libXi
AC_CHECK_LIB([Xmu], [main], [have_Xmu="yes"], [have_Xmu="no"])
AC_CHECK_LIB([Xi], [main], [have_Xi="yes"], [have_Xi="no"])

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

   AC_CHECK_HEADERS([gl.h glu.h glut.h glaux.h GL/gl.h GL/glu.h GL/glut.h GL/glaux.h OpenGL/gl.h OpenGL/glu.h OpenGL/glut.h OpenGL/glaux.h GLUT/glut.h MesaGL/gl.h MesaGL/glu.h MesaGL/glut.h MesaGL/glaux.h])

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
AC_CHECK_HEADERS(windows.h arpa/inet.h dirent.h fcntl.h inttypes.h stdint.h malloc.h alloca.h memory.h netdb.h netinet/in.h netinet/tcp.h signal.h strings.h sys/auxv.h sys/file.h sys/ipc.h sys/mount.h sys/param.h sys/resource.h sys/select.h sys/shm.h sys/socket.h sys/stat.h sys/statvfs.h sys/statfs.h sys/swap.h sys/sysctl.h sys/systeminfo.h sys/time.h sys/types.h sys/utsname.h sys/vmmeter.h sys/wait.h unistd.h utmp.h errno.h procfs.h ieeefp.h)

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
AC_CHECK_FUNCS(alloca _alloca setpriority strlcpy strlcat strcasestr sigaction getutent setutent getisax strdup strdupa daemon stat64 putenv setenv)

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

AM_OPTIONS_WXCONFIG

dnl check for wxWidgets

if test "${enable_client_release}" = yes ; then
    if test "${enable_debug}" = yes ; then
        if test "${enable_unicode}" = yes ; then
            echo 'Portable client-build: checking for debug unicode wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --static=yes --unicode=yes --debug=yes)
        else
            echo 'Portable client-build: checking for debug wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --static=yes --unicode=no --debug=yes)
        fi
    else
        if test "${enable_unicode}" = yes ; then
            echo 'Portable client-build: checking for unicode wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --static=yes --unicode=yes --debug=no)
        else
            echo 'Portable client-build: checking for wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --static=yes --unicode=no --debug=no)
        fi
    fi
else
    if test "${enable_debug}" = yes ; then
        if test "${enable_unicode}" = yes ; then
            echo 'checking for debug unicode wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=yes --debug=yes)
        else
            echo 'checking for debug wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=no --debug=yes)
        fi
    else
        if test "${enable_unicode}" = yes ; then
            echo 'checking for unicode wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=yes --debug=no)
        else
            echo 'checking for wx-libs ...'
            AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=no --debug=no)
        fi
    fi
fi


if ( test "${enable_client}" = yes ) && ( test "$wxWin" != 1 ); then
   AC_MSG_WARN([
================================================================================
WARNING: A suitable installation of wxWidgets could not be found 
	 ==> building client without clientgui.

  If you add wxWidgets to your system, then this configure script will also
  configure your system to build the BOINC graphical client (clientgui).  

  If wxWidgets is installed on your system, please check that wx-config is 
  in the path, that the directory where wxWidgets libraries are installed 
  (returned by 'wx-config --libs' command) is in LD_LIBRARY_PATH (or equivalent),
  and that wxWidgets version is 2.6.0 or above. Currently wxWidgets version
  2.6.0 (gtk based) is known to work with boinc_gui under Linux. You can
  use wx-config --version to find what version you have currently installed.

  NOTE: if building a portable client-release, you need the *static* version
  of the wx-libs installed!   

  You can get wxWidgets by following the DOWNLOAD link at:
  http://www.wxwindows.org/
================================================================================
       ])
fi

AM_CONDITIONAL(BUILD_CLIENTGUI, [ test "$wxWin" = 1 -a "${enable_client}" = yes ])

dnl --------------------------------------------------------------------------------

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

dnl tell automake about whether to build client and/or server
AM_CONDITIONAL(ENABLE_SERVER, [test "${enable_server}" = yes])
AM_CONDITIONAL(ENABLE_CLIENT, [test "${enable_client}" = yes])



dnl ======================================================================
dnl some more vodoo required for building portable client-binary (client, clientgui)
dnl ======================================================================

CLIENTLIBS=

SAH_CHECK_LIB([c],[atexit], [
    AC_DEFINE([HAVE_LIBC],[1],[Define to 1 if you have the c library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
SAH_CHECK_LIB([gcc_eh],[_Unwind_Resume],[ 
    AC_DEFINE([HAVE_LIBGCC_EH],[1],[Define to 1 if you have the gcc_eh library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
SAH_CHECK_LIB([stdc++],[sscanf],[
    AC_DEFINE([HAVE_LIBSTDC__],[1],[Define to 1 if you have the stdc++ library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
SAH_CHECK_LIB([gcc],[sscanf], [
    AC_DEFINE([HAVE_LIBGCC],[1],[Define to 1 if you have the gcc library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}" ])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
SAH_CHECK_LIB([m],[sin], [
    AC_DEFINE([HAVE_LIBM],[1],[Define to 1 if you have the math library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
SAH_CHECK_LIB([pthread],[pthread_join],[
    AC_DEFINE([HAVE_LIBPTHREAD],[1],[Define to 1 if you have the pthread library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
SAH_CHECK_LIB([wsock32],[fopen],[
    AC_DEFINE([HAVE_LIBWSOCK32],[1],[Define to 1 if you have the wsock32 library])
    CLIENTLIBS="${CLIENTLIBS} ${sah_lib_last}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
    

if ( test "${disable_static_linkage}" != yes ) && ( test "${enable_client_release}" = yes ); then

SAH_CHECK_LDFLAG([-nodefaultlibs],[CLIENTLIBS="-nodefaultlibs ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5

SAH_CHECK_LDFLAG([-lSystemStubs],[LIBS="${LIBS} -lSystemStubs"])
   echo DEBUG: LIBS=${LIBS} >&5

   echo "----------"
   echo "NOTE: Building portable client binaries"
   echo "----------"
   
   SAH_CHECK_LDFLAG([-nodefaultlibs],[CLIENTLIBS="-nodefaultlibs ${CLIENTLIBS}"])
   echo DEBUG: CLIENTLIBS=${CLIENTLIBS} >&5
        
   SAH_CHECK_LDFLAG([-lSystemStubs],[LIBS="${LIBS} -lSystemStubs"])
   echo DEBUG: LIBS=${LIBS} >&5

fi


AC_SUBST(CLIENTLIBS)

## --------------------
## some more tweaking to turn non-standard libs into statically linked ones

if test "${enable_debug}" = yes; then
    CLIENTGUIFLAGS="${CLIENTGUIFLAGS} -D_DEBUG -DDEBUG"
fi

CLIENTGUILIBS=${WX_LIBS}

if test "${enable_client_release}" = "yes" ; then
   SAH_STATICIZE_LDFLAGS([${WX_LIBS_STATIC}],[CLIENTGUILIBS])
   SAH_STATICIZE_LDFLAGS([${GLUT_LIBS}],[GLUT_LIBS])
   SAH_STATICIZE_LDFLAGS([${BOINC_EXTRA_LIBS}],[BOINC_EXTRA_LIBS])



   ## NOTE: its *extremely* important to always compile with __NO_CTYPE 
   ## defined, because this avoids GLIBC_2.3 symbols being pulled in via ctype.h-macros,
   CPPFLAGS="$CPPFLAGS -D__NO_CTYPE"
fi

## prepend BOINC_EXTRA_LIBS (mangled or not) to LIBS for standard linking
LIBS="${BOINC_EXTRA_LIBS} $LIBS"

AC_SUBST(CLIENTGUIFLAGS)
AC_SUBST(CLIENTGUILIBS)
dnl ======================================================================

AC_CONFIG_FILES([
                 ./version.h
                 api/Makefile
                 apps/Makefile
                 clientgui/Makefile
                 clientgui/res/Makefile
                 client/Makefile
                 client/win/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
		 client/scripts/Makefile
		 client/scripts/boinc-client
                 db/Makefile
                 doc/Makefile
                 doc/manpages/Makefile
                 lib/Makefile
		 locale/client/Makefile
                 Makefile
                 py/Boinc/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 py/Boinc/Makefile
                 py/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 py/Boinc/version.py
                 py/Makefile
                 sched/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 sched/Makefile
                 sea/Makefile
                 test/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 test/Makefile
                 test/version.inc
                 tools/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 tools/Makefile
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
