dnl -*- autoconf -*-

dnl $Id$

dnl NOTE: on milkyway.ssl and jill.ssl , need autoconf/automake et al from quarl's
dnl directory; the _autosetup script does this.

dnl not sure exactly what the minimum version is (but 2.13 won't work)
AC_PREREQ(2.57)

dnl Set the BOINC version here.  You can also use the set-version script.
AC_INIT(BOINC, 4.68)

AC_ARG_ENABLE(debug, 
    AC_HELP_STRING([--enable-debug],
                   [enable tracing and debugging flags for all components]),
    [enable_debug=yes],
    [])

AC_ARG_ENABLE(server, 
    AC_HELP_STRING([--disable-server],
                   [disable building the scheduling server]),
    [],
    [enable_server=yes])

AC_ARG_ENABLE(client, 
    AC_HELP_STRING([--disable-client],
                   [disable building the client]),
    [],
    [enable_client=yes])

m4_divert_once([HELP_ENABLE],
  AC_HELP_STRING([], [Default: --enable-server --enable-client: 
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

if test "${enable_debug}" = yes ; then
    build_state='Debug'
else
    build_state='Release'
fi


echo "--- Configuring BOINC AC_PACKAGE_VERSION (${build_state}) ---"
echo "--- Build Components: (${configured_to_build}) ---"

AC_CANONICAL_SYSTEM

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)
AM_INIT_AUTOMAKE(dist-zip)

AC_CONFIG_SRCDIR(lib/shmem.C)

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
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
])

AC_REVISION([$Revision$])

AC_DEFUN([KC_STRIP_LEADING_ZEROS],[m4_bregexp([$*],[^0*\(..*\)],\1)])

AC_DEFUN([BOINC_SET_VERSION],[dnl
AC_SUBST([BOINC_MAJOR_VERSION],KC_STRIP_LEADING_ZEROS([$1]))
AC_SUBST([BOINC_MINOR_VERSION],KC_STRIP_LEADING_ZEROS([$2]))
AC_SUBST([BOINC_VERSION_STRING],AC_PACKAGE_VERSION)
])

BOINC_SET_VERSION(translit(AC_PACKAGE_VERSION, [.], [,]))

dnl Version information moved to version.h so removed from config.h
dnl AC_DEFINE_UNQUOTED([BOINC_VERSION_STRING], "$BOINC_VERSION_STRING", [String representation of BOINC version number])
dnl AC_DEFINE_UNQUOTED([BOINC_MAJOR_VERSION], $BOINC_MAJOR_VERSION, [Major part of BOINC version number])
dnl AC_DEFINE_UNQUOTED([BOINC_MINOR_VERSION], $BOINC_MINOR_VERSION, [Minor part of BOINC version number])

dnl Need to duplicate these AC_SUBST because some versions of autoconf won't 
dnl find AC_SUBST in a macro unless subsequently used in a DEFINE or SUBST
dnl outside of a macro.  Don't ask me why.
AC_SUBST([BOINC_VERSION_STRING])
AC_SUBST([BOINC_MAJOR_VERSION])
AC_SUBST([BOINC_MINOR_VERSION])

AC_DEFINE_UNQUOTED([HOSTTYPE], "$host", [Host for this compilation])
AC_SUBST([CLIENT_BIN_FILENAME],[boinc])
AC_SUBST([CLIENT_GUI_BIN_FILENAME],[boincmgr])
AC_SUBST([TOP_BUILD_DIR], [`pwd`])
AC_SUBST([TOP_SOURCE_DIR], [`(cd \`dirname "$0"\` && pwd)`])

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LN_S

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
If you don't want to build the server you should use --disable-server.

I am continuing now as if --disable-server had been specified.

======================================================================
])
   enable_server=no
   fi
fi

dnl Checks for libraries.
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(cygipc, shmget)

dnl  NOTE: we might want to link the following libs statically 
dnl for building portable binaries, so we store them in a separate variable
dnl BOINC_EXTRA_LIBS for later post-processing instead of the default-action,
dnl which would prepend them to LIBS
dnl (note, we also don't define HAVE_LIBXXX, because currently they aren't used
AC_CHECK_LIB(nsl, gethostbyname, [BOINC_EXTRA_LIBS="-lnsl ${BOINC_EXTRA_LIBS}"])
AC_CHECK_LIB(socket, setservent, [BOINC_EXTRA_LIBS="-lsocket ${BOINC_EXTRA_LIBS}"])
AC_CHECK_LIB(z, gzopen, [BOINC_EXTRA_LIBS="-lz ${BOINC_EXTRA_LIBS}"])
AC_CHECK_LIB(dl, dlopen, [BOINC_EXTRA_LIBS="-ldl ${BOINC_EXTRA_LIBS}"])

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
AX_CHECK_GLUT
if test "$no_gl" = yes -o "$no_glu" = yes -o "$no_glut" = yes; then
   have_glut=no
   AC_MSG_WARN([
================================================================================
WARNING: openGL/GLU/glut not found.

The GL, GLU and glut libraries are required in order to build the graphical parts
of the BOINC application API library.

==> only building non-graphical parts of the BOINC API Library for now.

NOTE: on MacOS X/Darwin you might consider running configure withe the option
      './configure --with-apple-opengl-framework'
      in order to use the Mac-native openGL framework

================================================================================
       ])
else
   have_glut=yes
   echo "DEBUG: GLUT_CFLAGS = $GLUT_CFLAGS"
   echo "DEBUG: GLUT_LIBS = $GLUT_LIBS"

   AC_CHECK_HEADERS([gl.h glu.h glut.h glaux.h GL/gl.h GL/glu.h GL/glut.h GL/glaux.h OpenGL/gl.h OpenGL/glu.h OpenGL/glut.h OpenGL/glaux.h GLUT/glut.h MesaGL/gl.h MesaGL/glu.h MesaGL/glut.h MesaGL/glaux.h])
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
/* include fixes for the most common problems */
#include "std_fixes.h"

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
AC_CHECK_HEADERS(arpa/inet.h dirent.h fcntl.h malloc.h netdb.h netinet/in.h netinet/tcp.h signal.h strings.h sys/file.h sys/ipc.h sys/mount.h sys/param.h sys/resource.h sys/select.h sys/shm.h sys/socket.h sys/stat.h sys/statvfs.h sys/statfs.h sys/swap.h sys/sysctl.h sys/systeminfo.h sys/time.h sys/types.h sys/utsname.h sys/vmmeter.h sys/wait.h unistd.h utmp.h errno.h procfs.h)

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
dnl TODO: use compiler to test these; there probably exists an autoconf macro already!
case "$target" in
*-linux-* | *-sun-* | *openbsd* )
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
AC_CHECK_FUNCS(lockf flock setpriority wait4 strlcpy strlcat sigaction getutent setutent)

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

dnl Checks for device files
if test -e "/dev/mouse"; then
    AC_DEFINE(HAVE__DEV_MOUSE, 1, [Define to 1 if /dev/mouse exists])
fi
if test -e "/dev/kbd"; then
    AC_DEFINE(HAVE__DEV_KBD, 1, [Define to 1 if /dev/kbd exists])
fi
if test -e "/dev/tty1"; then
    AC_DEFINE(HAVE__DEV_TTY1, 1, [Define to 1 if /dev/tty1 exists])
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

if test "${enable_debug}" = yes ; then
    AM_PATH_WXCONFIG([2.3.4], [wxWin=1], [wxWin=0], --debug)
else
    AM_PATH_WXCONFIG([2.3.4], [wxWin=1], [wxWin=0])
fi

if ( test "${enable_client}" = yes ) && ( test "$wxWin" != 1 ); then
   AC_MSG_WARN([
================================================================================
WARNING: wxWidgets could not be found ==> building client without clientgui.
  If you add wxWidgets to your system, then this configure script will also
  configure your system to build the BOINC graphical client (clientgui).  

  If wxWidgets is installed on your system, please check that wx-config is 
  in the path, that the directory where wxWidgets libraries are installed 
  (returned by 'wx-config --libs' command) is in LD_LIBRARY_PATH (or equivalent),
  and that wxWidgets version is 2.3.4 or above. Currently wxWidgets version
  2.4.2 (gtk based) is known to work with boinc_gui under Linux. You can
  use wx-config --version to find what version you have currently installed.

  You can get wxWidgets by following the DOWNLOAD link at:
  http://www.wxwindows.org/
================================================================================
       ])
fi

AM_CONDITIONAL(BUILD_CLIENTGUI, [ test "$wxWin" = 1 -a "${enable_client}" = yes ])

dnl --------------------------------------------------------------------------------

dnl Here's another way to set up host-specific stuff
AM_CONDITIONAL(OS_DARWIN, [echo $host_os | grep '^darwin' > /dev/null])
AM_CONDITIONAL(OS_LINUX, [echo $host_os | grep '^linux' > /dev/null])
dnl In case anyone wants to try building the windows code using mingw!
AM_CONDITIONAL(OS_WIN32_MINGW, [echo $host_os | grep '^mingw' > /dev/null])

dnl tell automake about whether to build client and/or server
AM_CONDITIONAL(ENABLE_SERVER, [test "${enable_server}" = yes])
AM_CONDITIONAL(ENABLE_CLIENT, [test "${enable_client}" = yes])


dnl ======================================================================
dnl some vodoo required for building portable client-binary (client, clientgui)
dnl ======================================================================
AC_ARG_ENABLE([client-release], AS_HELP_STRING([--enable-client-release],
	[Try building a portable "release-candidate" (currently implemented for Linux and Solaris only): \
	this links libstd++ statically. You will probably need gcc-3.0 for 
	this to produce a portable client-binary. 
	It is therefore recommended to use CC=gcc-3.0 and CXX=g++-3.0 for this.
	(Default = no)]), [], [enable_client_release=no])

dnl Some platform specific settings
case $target in
   *linux* | *sun* ) 
	if ( test "${enable_client_release}" = yes ) && ( test "${enable_client}" != yes ); then
	   AC_MSG_WARN([--enable-client-release ignored.
--------------------------------------------------
The switch --enable-client-release is only useful for building the client and will be ignored
--------------------------------------------------]) 
        else
            LDSTATIC="-Wl,-Bstatic"
	    LDDYNAMIC="-Wl,-Bdynamic"
        fi
	;;
   *)	## in the non-linux, non-solaris case, --enable-client-release is not allowed
	if test "${enable_client_release}" = yes; then
	   AC_MSG_WARN([
----------------------------------------------------------------------
Sorry, the switch --enable-client-release is currently only implemented
for Linux and Solaris, and will have no effect.
----------------------------------------------------------------------])
	fi  
	LDSTATIC=
	LDDYNAMIC=
	;; 
esac

 ## we need to link IOKit.framework on MacOSX for the client-build
case $target in
   *darwin*)  
       if test "${enable_client}" = yes; then
           LIBS="$LIBS -framework IOKit"
       fi
       ;;
       *) ;;
esac

if ( test "${enable_client_release}" = yes ) && ( test "${enable_client}" = yes ); then
   echo "----------"
   echo "NOTE: Building portable client binaries"
   echo "----------"
   AC_CHECK_LIB(gcc_eh, _Unwind_Resume, [have_gcc_eh=yes], [have_gcc_eh=no])
   if test "$have_gcc_eh" = yes; then
      GCC_EH_LIB=-lgcc_eh
   else
      GCC_EH_LIB=
   fi

   CLIENTLIBS="-nodefaultlibs $LDSTATIC -lstdc++ -lgcc $GCC_EH_LIB $LDDYNAMIC -lpthread -lm  -lc"	
else
   CLIENTLIBS=
fi
AC_SUBST(CLIENTLIBS)

## --------------------
## some more tweaking to turn non-standard libs into statically linked ones
CLIENTGUIFLAGS="-DNOCLIPBOARD -DNOTASKBAR"

if test "${enable_debug}" = yes; then
    CLIENTGUIFLAGS="${CLIENTGUIFLAGS} -D_DEBUG -DDEBUG"
fi

CLIENTGUILIBS=${WX_LIBS}

if test "${enable_client_release}" = yes; then
   ## list of possible libraries, which we have to link statically against 
   ## in order to avoid portability problems
      
   ## extend this list as necessary if you find additional libs required on your system
   ## ****************************************
   blacklist="gtk gdk gmodule gthread glib Xi png jpeg tiff nsl socket"
   case $target in 
      *linux*)
         blacklist="$blacklist z"
         ;;
      *)
         ;;
   esac
   ## ****************************************

   ## now we walk through the blacklist and turn all matching libs found in 
   ## WX_LIBS_STATIC into a static link-command
   CLIENTGUILIBS="-L. ${WX_LIBS_STATIC} -L."  ## dummy -L's to make sure the regexp always works
   echo "DEBUG: before mangling CLIENTGUILIBS='$CLIENTGUILIBS'" 
   for somelib in ${blacklist}; do
       CLIENTGUILIBS=`echo "$CLIENTGUILIBS" | sed -e "s/ -l${somelib} / $LDSTATIC -l${somelib} $LDDYNAMIC /g"`
   done
   echo "DEBUG: final link-line for CLIENTGUILIBS = $CLIENTGUILIBS"

   ## do exactly the same for all BOINC_EXTRA_LIBS
   BOINC_EXTRA_LIBS="-L. ${BOINC_EXTRA_LIBS} -L."    ## dummy -L's to make sure the regexp always works
   echo "DEBUG: before mangling BOINC_EXTRA_LIBS='$BOINC_EXTRA_LIBS'"
   for somelib in ${blacklist}; do
       BOINC_EXTRA_LIBS=`echo "$BOINC_EXTRA_LIBS" | sed -e "s/ -l${somelib} / $LDSTATIC -l${somelib} $LDDYNAMIC /g"`
   done
   echo "DEBUG: final BOINC_EXTRA_LIBS= $BOINC_EXTRA_LIBS"


   ## NOTE: it's *extremely* important to always compile with __NO_CTYPE 
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
                 client/Makefile
                 client/win/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 db/Makefile
                 lib/Makefile
                 Makefile
                 py/Boinc/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 py/Boinc/Makefile
                 py/boinc_path_config.py:py/Boinc/boinc_path_config.py.in
                 py/Boinc/version.py
                 py/Makefile
                 RSAEuro/Makefile
                 RSAEuro/source/Makefile
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

if test "${enable_server}" = yes ; then
  cat<<EOF

We recommend you run test/test_sanity.py to make sure your host is configured
properly for building the BOINC server.

EOF
fi

]
