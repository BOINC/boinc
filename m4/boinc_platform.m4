AC_DEFUN([BOINC_PLATFORM],[
  AC_ARG_WITH([boinc-platform],
              AC_HELP_STRING([--with-boinc-platform],
	                      [override the default boinc platform]),
	      [boinc_platform="$withval"],
	      [boinc_platform=])
  AC_ARG_WITH([boinc-alt-platform],
              AC_HELP_STRING([--with-boinc-alt-platform],
	                      [override the boinc alternate platform]),
	      [boinc_alt_platform="$withval"],
	      [boinc_alt_platform=])
  AC_MSG_CHECKING([boinc platform])
  if test -z "${boinc_platform}" ; then
    boinc_platform=`echo $target | $SED -e 's/amd64/x86_64/' -e 's/portbld/pc/' -e 's/redhat/pc/' -e 's/x86_64-unknown/x86_64-pc/' -e 's/[[0-9]]$//' -e 's/[[0-9]]$//' -e 's/\.$//' -e 's/[[0-9]]$//' -e 's/\.$//' -e 's/[[0-9]]$//'`
    case "${boinc_platform}" in
      sparc-sun-solaris)
    	if test "$COMPILER_MODEL_BITS" = "64" ; then
	  boinc_platform=`echo $boinc_platform | $SED 's/sparc/sparc64/'`
	  if test -z "$boinc_alt_platform" ; then
	    boinc_alt_platform=sparc-sun-solaris
	  fi
	elif test -z "$boinc_alt_platform" ; then
	    boinc_alt_platform=sparc-sun-solaris2.7
	fi
        ;;
      x86_64*linux-gnu)
        if test "$COMPILER_MODEL_BITS" = "32" ; then
	  boinc_platform="i686-pc-linux-gnu"
	elif test -z "$boinc_alt_platform" ; then
	  boinc_alt_platform="i686-pc-linux-gnu"
	fi
	;;
      powerpc-apple-darwin)
        if test "$COMPILER_MODEL_BITS" = "64" ; then
	  boinc_platform="powerpc64-apple-darwin"
	  if test -z "$boinc_alt_platform" ; then
	    boinc_alt_platform="powerpc-apple-darwin"
	  fi
	fi
	;;
      hppa*-hp-hpux*)
        if test "$COMPILER_MODEL_BITS" = "64" ; then
	  boinc_platform="hppa64-hp-hpux"
	  if test -z "${boinc_alt_platform}" ; then
	    boinc_alt_platform="hppa-hp-hpux"
	  fi
	else
	  boinc_platform="hppa-hp-hpux"
	fi
        ;;
      ia64-hp-hpux*)
	boinc_platform="ia64-hp-hpux"
        ;;
    esac
    case "${target}" in
      i386-pc-os2)
        boinc_platform="i686-pc-os2"
        ;;
    esac
  fi
  AC_DEFINE_UNQUOTED([HOSTTYPE],"$boinc_platform",[Platform identification used to identify applications for this BOINC core client])
  AC_SUBST([boinc_platform],$boinc_platform)
  AC_MSG_RESULT([$boinc_platform])
  AC_MSG_CHECKING([alternate boinc platform])
  if test -n "$boinc_alt_platform" ; then
     AC_DEFINE_UNQUOTED([HOSTTYPEALT],"$boinc_alt_platform",[Alternate identification used to identify applications for this BOINC core client])
     AC_SUBST([boinc_alt_platform],$boinc_alt_platform)
     AC_MSG_RESULT($boinc_alt_platform)
  else
     AC_MSG_RESULT(none)
  fi
])
    
