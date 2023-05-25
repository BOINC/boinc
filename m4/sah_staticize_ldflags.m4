AC_DEFUN([SAH_STATICIZE_LDFLAGS],[
   STATIC_LIB_LIST="${STATIC_LIB_LIST} $3"
   liblist=`echo $1 | $AWK '{for (i=1;i<(NF+1);i++) {print $[]i;}}' | grep -v "Wl,[sd]" `
   ssl_sah_save_libs="${LIBS}"
   echo "DEBUG: before mangling $2:$1" >&5
   sah_outputlibs=
   for somelib in ${liblist}; do
       # look for the -l to find the libraries
       alib=`echo x${somelib} | grep x-l | sed 's/x-l//'`
       if test -n "${alib}"
       then
         # check to see if it is in our static list
         for slib in ${STATIC_LIB_LIST}; do
	   lib_is_static="no"
	   tmp_pattern=`echo s/x${slib}// | sed 's/\*/.*/'`
	   if test -z "`echo x${alib} | sed ${tmp_pattern}`"
	   then
             SAH_STATIC_LIB(${alib},[fopen],[sah_outputlibs="${sah_outputlibs} ${sah_lib_last}"])
	     lib_is_static="yes"
	     break;
	   fi
	 done
	 if test "${lib_is_static}" = "no" ; then
	   SAH_DYNAMIC_LIB(${alib},[fopen],[sah_outputlibs="${sah_outputlibs} ${sah_lib_last}"])
	 fi
       else
         tmp_pattern_a="s/x${ld_dynamic_option}//"
         tmp_pattern_b="s/x${ld_static_option}//"
         if test -n "`echo x${somelib} | sed ${tmp_pattern_a}`" -a \
	         -n "`echo x${somelib} | sed ${tmp_pattern_b}`"
	 then
	    sah_outputlibs="${sah_outputlibs} ${somelib}"
	    LIBS="${LIBS} ${somelib}"
         fi
       fi
   done
   echo "DEBUG: final link-line for $2:${sah_outputlibs}" >&5
   $2=${sah_outputlibs}
   LIBS="${ssl_sah_save_libs}"
])

