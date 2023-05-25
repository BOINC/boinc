
AC_DEFUN([SAH_CHECK_LIB],[
   alib="$1"
   # check to see if it is in our static list
   for slib in ${STATIC_LIB_LIST}; do
     lib_is_static="no"
     tmp_pattern=`echo s/x${slib}// | sed 's/\*/.*/'`
     if test -z "`echo x${alib} | sed ${tmp_pattern}`"
     then
       SAH_STATIC_LIB_REQUIRED(${alib},[$2],[
         lib_is_static="yes"
         sah_lib_last="${sah_static_lib_last}"
	 $3
       ],[$4],[$5])
       break;
     fi
   done
   if test "${lib_is_static}" = "no" ; then
     SAH_DYNAMIC_LIB_REQUIRED(${alib},[$2],[
         sah_lib_last="${sah_dynamic_lib_last}"
	 $3
     ],[$4], [$5])
   fi
])



AC_DEFUN([SAH_CHECK_LDFLAG],[
    sv_ldflags="${LDFLAGS}"
    AC_MSG_CHECKING(if compiler works with $1 flag)
    LDFLAGS="${LDFLAGS} $1"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[
        #define CONFIG_TEST
         int foo() {return 1;}
      ]],
      [ return foo(); ])],
      [
        AC_MSG_RESULT(yes)
	$2
      ],
      [
        AC_MSG_RESULT(no)
	LDFLAGS="${sv_ldflags}"
	$3
      ]
    )
])

AC_DEFUN([SAH_LINKAGE_FLAGS],[
  if test "${disable_static_linkage}" = "yes"
  then
    ld_static_option=""
    ld_dynamic_option=""
    LD_EXPORT_DYNAMIC=""
  else
    if test -z "${ld_static_option}" && test -z "${using_libtool}"
    then
      AC_MSG_CHECKING([if we are using libtool])
      if test "x$lt_cv_path_LD" != "x" ; then
        AC_MSG_RESULT(yes)
	using_libtool=yes
	ld_static_option="-static-libtool-libs"
      else
        AC_MSG_RESULT(no)
	using_libtool=no
        case $target in
          *linux* | *solaris* | *cygwin* )
            AC_MSG_CHECKING([${CC} flags for static linkage ...])
	    ld_static_option="-Wl,-Bstatic"
	    AC_MSG_RESULT($ld_static_option)
            AC_MSG_CHECKING([${CC} flags for dynamic linkage ...])
	    ld_dynamic_option="-Wl,-Bdynamic"
	    AC_MSG_RESULT($ld_dynamic_option)
            ;;
          *darwin* )
            AC_MSG_CHECKING([${CC} flags for static linkage ...])
	    ld_static_option="-static"
	    AC_MSG_RESULT($ld_static_option)
            AC_MSG_CHECKING([${CC} flags for dynamic linkage ...])
	    ld_dynamic_option="-dynamic"
	    AC_MSG_RESULT($ld_dynamic_option)
            ;;
	  *)
	    if test -z "${dummy_ld_variable_gfdsahjf}"
	    then
	      dummy_ld_variable_gfdsahjf="been there done that"
              AC_MSG_CHECKING([${CC} flags for static linkage ...])
	      AC_MSG_RESULT(unknown)
              AC_MSG_CHECKING([${CC} flags for dynamic linkage ...])
	      AC_MSG_RESULT(unknown)
            fi
	    ;;
        esac
        AC_MSG_CHECKING([${CC} flags for exporting dynamic symbols from an executable ...])
        case $target in
          *cygwin*)
	     LD_EXPORT_DYNAMIC="-Wl,--export-all-symbols"
	     AC_MSG_RESULT(${LD_EXPORT_DYNAMIC})
	     ;;
          *linux*)
	     AC_MSG_RESULT(-rdynamic)
	     LD_EXPORT_DYNAMIC="-rdynamic"
	     ;;
	  *)
	     AC_MSG_RESULT(none required)
	     LD_EXPORT_DYNAMIC=
	     ;;
        esac
      fi
    fi
  fi
  if test -z "${LIBEXT}";
  then
    SAH_LIBEXT
  fi
  if test -z "${DLLEXT}";
  then
    SAH_DLLEXT
  fi
  AC_SUBST(LD_EXPORT_DYNAMIC)
])


# use this function in order to find a library that we require to be static.
# we will check in the following order....
#    1) files named lib{name}.a
#    2) files named {name}.a
#    3) linking with the static flags "$STATIC_FLAGS -l{name}"
AC_DEFUN([SAH_STATIC_LIB_REQUIRED],[
SAH_LINKAGE_FLAGS
# uppercase the library name for our definition
ac_uc_defn=HAVE_LIB`echo $1 | sed -e 's/[^a-zA-Z0-9_]/_/g' \
    -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/'`
# Build our cache variable names
STATIC_LIB_LIST="${STATIC_LIB_LIST} $1"
varname=`echo sah_cv_static_lib_$1_$2 | $as_tr_sh`
var2=`echo sah_cv_static_lib_$1_fopen | $as_tr_sh`
if test "$2" != "fopen" ; then
  tmp_msg="for $2 in static library $1"
else
  tmp_msg="for static library $1"
fi
AC_CACHE_CHECK([$tmp_msg],
  [$varname],
[
  tmp_res="no"
#
# check if we want to actually do all this.
  if test "${disable_static_linkage}" = "yes"
  then
    sah_static_checklibs="-l$1"
  else
    sah_static_checklibs="lib$1.${LIBEXT} $1.${LIBEXT} -l$1"
  fi
  if test "${using_libtool}" = "yes" ; then
    ac_sv_link="$ac_link"
    ac_link="./libtool --mode=link ${ac_link}"
    sah_static_checklibs="lib$1.la $1.la ${sah_static_checklibs}"
  fi
  sah_save_libs="${LIBS}"
  for libname in ${sah_static_checklibs}
  do
    SAH_FIND_STATIC_LIB(${libname})
dnl    if test -z "`echo $tmp_lib_name | grep \\.la$`"  ; then
      if test -n "${tmp_lib_name}"
      then
        LIBS="${tmp_lib_name} $5 ${sah_save_libs}"
        AC_LINK_IFELSE([
          AC_LANG_PROGRAM([[
            #define CONFIG_TEST 1
	    #ifdef __cplusplus
	    extern "C" {
	    #endif
            char $2 ();
	    #ifdef __cplusplus
	    }
	    #endif
          ]],
          [ $2 (); ])],
          [
            tmp_res="${tmp_lib_name}"
          ]
        )
      fi
dnl    else
dnl      tmp_res="-l$1"
dnl    fi
    if test "${tmp_res}" != "no"
    then
      break
    fi
  done
  LIBS="${sah_save_libs}"
  eval ${varname}='"'${tmp_res}'"'
  eval ${var2}='"'${tmp_res}'"'
  ])
  if test "${using_libtool}" = "yes" ; then
    ac_link="$ac_sv_link"
  fi
#
# save the result for use by the caller
sah_static_lib_last="`eval echo '${'$varname'}'`"
#
if test "${sah_static_lib_last}" != "no"
then
  LIBS="${sah_static_lib_last} ${LIBS}"
  AC_DEFINE_UNQUOTED([$ac_uc_defn], [1],
    [Define to 1 if the $1 library has the function $2]
  )
  $3
else
  $4
  echo > /dev/null
fi
])

# use this function in order to find a library that we require to be dynamic.
# we will check in the following order....
#    1) linking with the dynamic flags "$DYNAMIC_FLAGS -l{name}"
#    2) linking with no flags "-l{name}"
AC_DEFUN([SAH_DYNAMIC_LIB_REQUIRED],[
SAH_LINKAGE_FLAGS
# upercase the library name for our definition
ac_uc_defn=HAVE_LIB`echo $1 | sed -e 's/[^a-zA-Z0-9_]/_/g' \
    -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/'`
# Build our cache variable names
DYNAMIC_LIB_LIST="${DYNAMIC_LIB_LIST} $1"
varname=`echo sah_cv_dynamic_lib_$1_$2 | $as_tr_sh`
var2=`echo sah_cv_dynamic_lib_$1_fopen | $as_tr_sh`
if test "$2" != "fopen" ; then
  tmp_msg="for $2 in dynamic library $1"
else
  tmp_msg="for dynamic library $1"
fi
AC_CACHE_CHECK([$tmp_msg],
  [$varname],
[
  tmp_res="no"
#
# check if we want to actually do all this.
  if test "${disable_static_linkage}" = "yes"
  then
    sah_dynamic_checklibs="-l$1 -l$1.${DLLEXT}"
  else
    sah_dynamic_checklibs="-l$1"
  fi
  if test "${using_libtool}" = "yes" ; then
    ac_sv_link="$ac_link"
    ac_link="./libtool --mode=link ${ac_link}"
    sah_dynamic_checklibs="lib$1.la $1.la ${sah_dynamic_checklibs}"
  fi
  sah_save_libs="${LIBS}"
  for libname in ${sah_dynamic_checklibs}
  do
    tmp_lib_name="${libname}"
    LIBS="${ld_dynamic_option} ${tmp_lib_name} $5 ${sah_save_libs}"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[
        #define CONFIG_TEST 1
	#ifdef __cplusplus
	extern "C" {
	#endif
        char $2 ();
	#ifdef __cplusplus
	}
	#endif
      ]],
      [ $2 (); ])
    ],
    [
      tmp_res="${ld_dynamic_option} ${tmp_lib_name}"
    ])
    if test "${tmp_res}" = "no"
    then
      tmp_lib_name="${libname}"
      LIBS="${tmp_lib_name} ${sah_save_libs}"
      AC_LINK_IFELSE([
        AC_LANG_PROGRAM([[
          #define CONFIG_TEST 1
	  #ifdef __cplusplus
	  extern "C" {
	  #endif
          char $2 ();
	  #ifdef __cplusplus
	  }
	  #endif
        ]],
        [ $2 (); ])
      ],
      [
        tmp_res="${tmp_lib_name}"
      ])
    fi
  done
  if test "${using_libtool}" = "yes" ; then
    ac_link="${ac_sv_link}"
  fi
  LIBS="${sah_save_libs}"
  eval ${varname}='"'${tmp_res}'"'
  eval ${var2}='"'${tmp_res}'"'
])
#
# save the result for use by the caller
sah_dynamic_lib_last="`eval echo '${'$varname'}'`"
#
if test "${sah_dynamic_lib_last}" != "no"
then
  LIBS="${sah_dynamic_lib_last} ${LIBS}"
  AC_DEFINE_UNQUOTED([$ac_uc_defn], [1],
    [Define to 1 if the $1 library has the function $2]
  )
  $3
else
  $4
  echo > /dev/null
fi
])

AC_DEFUN([SAH_STATIC_LIB],[
  SAH_STATIC_LIB_REQUIRED([$1],[$2])
  sah_lib_last="${sah_static_lib_last}"
  if test "${sah_lib_last}" = "no" ; then
    SAH_DYNAMIC_LIB_REQUIRED([$1],[$2])
    sah_lib_last=${sah_dynamic_lib_last}
  fi
  if test "${sah_lib_last}" != "no" ; then
    $3
    echo > /dev/null
  else
    $4
    echo > /dev/null
  fi
])

AC_DEFUN([SAH_DYNAMIC_LIB],[
  SAH_DYNAMIC_LIB_REQUIRED([$1],[$2])
  sah_lib_last="${sah_dynamic_lib_last}"
  if test "${sah_lib_last}" = "no" ; then
    SAH_STATIC_LIB_REQUIRED([$1],[$2])
    sah_lib_last=${sah_static_lib_last}
  fi
  if test "${sah_lib_last}" != "no" ; then
    $3
    echo > /dev/null
  else
    $4
    echo > /dev/null
  fi
])

#The SAH_FIND_STATIC_LIB macro searches the LD_LIBRARY_PATH or equivalent
#in order to find a static version of the library being loaded.
AC_DEFUN([SAH_FIND_STATIC_LIB],[
# libtool sets up the variable shlibpath_var which holds the name of the
# LIB_PATH variable.  We also want to strip the sparcv9 and 64s from the
# path, because we'll add them again later
strip_pattern="s/sparcv9//g; s/lib64/lib/g; s/lib\/64/lib/g"
tmp_libpath=`eval echo '${'$shlibpath_var'}' | sed "${strip_pattern}"`

# in cygwin, the DLLs are in the path, but the static libraries are elsewhere.
# Here's an educated guess.
if test "${shlibpath_var}" = "PATH"
then
  tmp_libpath=`echo ${PATH} | sed 's/\/bin/\/lib/g'`
  tmp_libpath="${tmp_libpath}:${PATH}"
fi



gcc_version=`${CC} -v 2>&1 | grep "gcc version" | $AWK '{print $[]3}'`


for gcc_host in `${CC} -v 2>&1 | grep host` --host=${ac_cv_target}
do
  if test -n "`echo x$gcc_host | grep x--host=`"
  then
    gcc_host="`echo $gcc_host | sed 's/--host=//'`"
    break
  fi
done

for dirs in `${CC} -v 2>&1 | grep prefix` --prefix=/usr/local
do
  if test -n "`echo x$dirs | grep x--prefix=`"
  then
    gcc_prefix="`echo $dirs | sed 's/--prefix=//'`"
    gcc_specs=`${CC} -v 2>&1 | grep specs | $AWK '{print $[]4}' | sed 's/\/specs//'`
    tmp_libpath="${gcc_specs}:${gcc_prefix}/lib:${gcc_prefix}/lib/gcc-lib/${gcc_host}/${gcc_version}:${tmp_libpath}"
    break
  fi
done

mydir=`pwd`
tmpgcc_libpath=

for dirs in `${CC} -print-search-dirs 2>&1 | grep libraries | sed -e 's/:/ /g' -e 's/=/ /g'`
do
  if test -d $dirs/. && cd $dirs 2>&1 >/dev/null ; then
    nowdir=`pwd`
    if test -n "${tmpgcc_libpath}" ; then
      tmpgcc_libpath="${tmpgcc_libpath}:${nowdir}"
    else
      tmpgcc_libpath="${dirs}"
    fi
  fi
  cd $mydir
done
cd $mydir
if test -n "${tmpgcc_libpath}" ; then
  tmp_libpath="${tmpgcc_libpath}:${tmp_libpath}"
fi

# now lets add any directories in LIBS or LDFLAGS
tmp_more_dirs=`echo $LIBS $LDFLAGS | $AWK '{for (i=1;i<(NF+1);i++) { printf("x%s\n",$[]i); }}' | grep x-L | sed 's/x-L//' | $AWK '{printf("%s:",$[]1);}'`
tmp_libpath="${tmp_more_dirs}${tmp_libpath}"


# Put machine/arch specific tweaks to the libpath here.
if test -z "${COMPILER_MODEL_BITS}"
then
  SAH_DEFAULT_BITNESS
fi
case $target in
  x86_64-*-linux*)
  	if test -n "${COMPILER_MODEL_BITS}"
	then
	  tmp_pattern="s/\/lib/\/lib${COMPILER_MODEL_BITS}/g"
	  tmp_pattern_b="s/${COMPILER_MODEL_BITS}${COMPILER_MODEL_BITS}/${COMPILER_MODEL_BITS}/g"
  	  abcd_q=`echo ${tmp_libpath} | sed ${tmp_pattern} | sed ${tmp_pattern_b} `
	  case ${COMPILER_MODEL_BITS} in
	    32)
	       gcc_host_dirs=`echo ${gcc_prefix}/lib/gcc*/i[3456]86*linux*/${gcc_version}`
	       ;;
	    64)
	       gcc_host_dirs=`echo ${gcc_prefix}/lib/gcc-lib/x86_64*linux*/${gcc_version}`
	       ;;
	  esac
	  tmp_libpath="${abcd_q}:${tmp_libpath}"
	  for tmp_dir in ${gcc_host_dirs}
	  do
	    tmp_libpath="${tmp_dir}:${tmp_libpath}"
	  done
	fi
	;;
  sparc*-sun-solaris*)
        if test -n "${COMPILER_MODEL_BITS}"
	then
	  tmp_pattern="s/\/lib/\/lib\/${COMPILER_MODEL_BITS}/g"
	  tmp_pattern_b="s/${COMPILER_MODEL_BITS}\/${COMPILER_MODEL_BITS}/${COMPILER_MODEL_BITS}/g"
  	  abcd_q=`echo ${tmp_libpath} | sed ${tmp_pattern} | sed ${tmp_pattern_b}`
	  case ${COMPILER_MODEL_BITS} in
	    64)
	        tmp_arch="sparcv9"
	        ;;
	    32)
	        tmp_arch=
		;;
          esac
	  abcd_r=
	  for tmp_dir in `echo ${tmp_libpath} | sed 's/\:/ /g'`
	  do
	    abcd_r="${abcd_r}:${tmp_dir}/${tmp_arch}"
	  done
	  tmp_libpath="${abcd_r}:${abcd_q}:${tmp_libpath}"
        fi
	;;
  *)
        echo > /dev/null
	;;
esac


if test -n "`echo x$1 | grep x-l`"
then
  # in the -l case, don't search, just use the ld_static_option (usually
  # -Wl,-B static
  tmp_lib_name="${ld_static_option} $1"
else
  # we also want to check the system config files for library dirs.
  tmp_dir_list=
  if test -e /etc/ld.so.conf
  then
    tmp_dir_list=`cat /etc/ld.so.conf`
  else
    if test -e /var/ld/ld.config
    then
      tmp_dir_list=`cat /var/ld/ld.config`
    fi
  fi

  ## add library-paths from LDFLAGS to beginning of lib-path
  ldflags_path=`echo $LDFLAGS | sed 's/-l[[^ ]]*//g' | sed 's/-L\([[^ ]]*\)/:\1/g' | sed 's/[[ ]]*//g'`

  tmp_dir_list=`echo ${ldflags_path}:${tmp_libpath}:/lib:/usr/lib:/usr/ucb/lib:/usr/local/lib:/opt/misc/lib:${tmp_dir_list} | $AWK -F: '{for (i=1;i<(NF+1);i++) { print $[]i; }}'`

  tmp_lib_name=
  # now that we know where we are looking, find our library
  for tmp_dir in $tmp_dir_list
  do
    if test -e $tmp_dir/$1
    then
      tmp_lib_name=${tmp_dir}/$1
      break
    fi
  done
fi
])
