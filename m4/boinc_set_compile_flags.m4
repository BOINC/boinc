AC_DEFUN([BOINC_SET_COMPILE_FLAGS],[
dnl  This function sets the compiler flags depending upon options
dnl  set on the configure command line.

AC_ARG_ENABLE(debug, 
    AS_HELP_STRING([--enable-debug],
                   [enable tracing and debugging flags for all components]),
    [enable_debug="$enableval"],
    [])

AC_ARG_ENABLE(optimize, 
    AS_HELP_STRING([--enable-optimize],
                   [enable optimization flags for all components]),
    [enable_optimize="$enableval"],
    [])

AC_ARG_ENABLE(generic-processor, 
    AS_HELP_STRING([--enable-generic-processor],
                   [build for a generic processor rather than a specific instruction set]),
    [enable_generic_processor="$enableval"],
    [])


dnl enable debug mode on all components using standard debug flags

if test x${enable_debug} = xyes ; then
  BOINC_CHECK_CFLAG(-g)
  BOINC_CHECK_CXXFLAG(-g)
  BOINC_CHECK_FFLAG(-g)
  BOINC_CHECK_LDFLAG(-g)
  CXXFLAGS="$CXXFLAGS -DDEBUG -D_DEBUG"
  CFLAGS="$CFLAGS -DDEBUG -D_DEBUG"
fi  

if test x${enable_optimize} = xyes ; then
  if test x${ac_cv_c_compiler_gnu} = xyes ; then
    BOINC_CHECK_CFLAG(-ffast-math)
  else
    BOINC_CHECK_CFLAG(-fast)
  fi
  BOINC_CHECK_CFLAG(-O3)
  if test x${ac_cv_cxx_compiler_gnu} = xyes ; then
    BOINC_CHECK_CXXFLAG(-ffast-math)
  else
    BOINC_CHECK_CXXFLAG(-fast)
  fi
  BOINC_CHECK_CXXFLAG(-O3)
  if test x${ac_cv_f77_compiler_gnu} = xyes ; then
    BOINC_CHECK_FFLAG(-ffast-math)
  else
    BOINC_CHECK_FFLAG(-fast)
  fi
  BOINC_CHECK_FFLAG(-O3)
fi  

if test x${enable_generic_processor} = xyes ; then
   case ${target} in
     i*86-*-darwin*) 
         BOINC_CHECK_CFLAG(-march=pentium4)
	 BOINC_CHECK_CFLAG(-mtune=prescott)
         BOINC_CHECK_CXXFLAG(-march=pentium4)
	 BOINC_CHECK_CXXFLAG(-mtune=prescott)
         BOINC_CHECK_FFLAG(-march=pentium4)
	 BOINC_CHECK_FFLAG(-mtune=prescott)
	 ;;
     i*86-*)  
         dnl gcc
	 if test x${ac_cv_c_compiler_gnu} = xyes ; then
           BOINC_CHECK_CFLAG(-march=i486)
           BOINC_CHECK_CFLAG(-mtune=generic)
         else
         dnl sun studio
           BOINC_CHECK_CFLAG(-xarch=386)
	 dnl dmc
	   BOINC_CHECK_CFLAG(-3)
	 fi
	 if test x${ac_cv_cxx_compiler_gnu} = xyes ; then
           BOINC_CHECK_CXXFLAG(-march=i486)
           BOINC_CHECK_CXXFLAG(-mtune=generic)
         else
           BOINC_CHECK_CXXFLAG(-xarch=386)
	   BOINC_CHECK_CXXFLAG(-3)
	 fi
	 if test x${ac_cv_f77_compiler_gnu} = xyes ; then
           BOINC_CHECK_FFLAG(-march=i486)
           BOINC_CHECK_FFLAG(-mtune=generic)
         else
           BOINC_CHECK_FFLAG(-xarch=386)
	   BOINC_CHECK_FFLAG(-3)
	 fi
	 ;;
     x86_64-*|amd64-*)  
         dnl gcc
	 if test x${ac_cv_c_compiler_gnu} = xyes ; then
           BOINC_CHECK_CFLAG(-msse2)   
           BOINC_CHECK_CFLAG(-march=opteron)   
           BOINC_CHECK_CFLAG(-mtune=generic)   
         else
           BOINC_CHECK_CFLAG(-xarch=amd64)   
         fi
	 if test x${ac_cv_cxx_compiler_gnu} = xyes ; then
           BOINC_CHECK_CXXFLAG(-msse2)   
           BOINC_CHECK_CXXFLAG(-march=opteron)   
           BOINC_CHECK_CXXFLAG(-mtune=generic)   
         else
           BOINC_CHECK_CXXFLAG(-xarch=amd64)
         fi
	 if test x${ac_cv_f77_compiler_gnu} = xyes ; then
           BOINC_CHECK_FFLAG(-msse2)   
           BOINC_CHECK_FFLAG(-march=opteron)   
           BOINC_CHECK_FFLAG(-mtune=generic)   
         else
           BOINC_CHECK_FFLAG(-xarch=amd64)
         fi
	 ;;
     sparc-*)
         dnl gcc
	 if test x${ac_cv_c_compiler_gnu} = xyes ; then
           BOINC_CHECK_CFLAG(-march=v8)   
           BOINC_CHECK_CFLAG(-mcpu=v8)   
           BOINC_CHECK_CFLAG(-mtune=ultrasparc)   
         else
	 dnl Studio 10
           BOINC_CHECK_CFLAG(-xarch=v8)   
         fi
	 if test x${ac_cv_cxx_compiler_gnu} = xyes ; then
           BOINC_CHECK_CXXFLAG(-march=v8)   
           BOINC_CHECK_CXXFLAG(-mcpu=v8)   
           BOINC_CHECK_CXXFLAG(-mtune=ultrasparc)   
         else
           BOINC_CHECK_CXXFLAG(-xarch=v8)   
         fi
	 if test x${ac_cv_f77_compiler_gnu} = xyes ; then
           BOINC_CHECK_FFLAG(-march=v8)   
           BOINC_CHECK_FFLAG(-mcpu=v8)   
           BOINC_CHECK_FFLAG(-mtune=ultrasparc)   
         else
           BOINC_CHECK_FFLAG(-xarch=v8)   
         fi
         ;;
     *)
         ;;
  esac
fi
])
