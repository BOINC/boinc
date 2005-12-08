AC_DEFUN([BOINC_PLATFORM],[
  AC_ARG_WITH([boinc-platform],
              AC_HELP_STRING([--with-boinc-platform],
	                      [override the default boinc platform]),
	      [boinc_platform="$withval"],
	      [boinc_platform=])
  AC_MSG_CHECKING([boinc platform])
  if test -z "${boinc_platform}" ; then
    boinc_platform=`echo $target | $SED -e 's/redhat/pc/' -e 's/[0-9]$//' -e 's/[0-9]$//' -e 's/\.$//' -e 's/[0-9]$//'`
    case "${boinc_platform}" in
      sparc-sun-solaris)
    	if test "$COMPILER_MODEL_BITS" = "64" ; then
	  boinc_platform=`echo $boinc_platform | $SED 's/sparc/sparc64/'`
	fi
        ;;
      x86_64*linux-gnu)
        if test "$COMPILER_MODEL_BITS" = "32" ; then
	  boinc_platform="i686-pc-linux-gnu"
	fi
	;;
    esac
  fi
  AC_DEFINE_UNQUOTED([HOSTTYPE],"$boinc_platform",[Platform identification used to identify applications for this BOINC core client])
  AC_MSG_RESULT($boinc_platform)
])
    
