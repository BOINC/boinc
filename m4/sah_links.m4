AC_DEFUN([SAH_LINKS],[
  AC_PATH_PROGS(LN,[ln cp copy])
  if test -n "$LN" ; then
    AC_MSG_CHECKING(whether '$LN' works)
    mkdir -p lntest
    cp ${ac_aux_dir}/config.sub lntest/
    if $LN lntest/config.sub erase.me$$ && \
       test -e erase.me$$ && \
       diff lntest/config.sub erase.me$$ >/dev/null 2>&5
    then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
      LN=cp
    fi
    /bin/rm erase.me$$ lntest/config.sub
    /bin/rmdir lntest
  else
    LN=cp
  fi
  AC_PROG_LN_S
  if test -n "$LN_S" ; then
    AC_MSG_CHECKING(whether '$LN_S' really works or whether I'm deluding myself)
    if $LN_S ${ac_aux_dir}/config.sub erase.me$$ && \
       test -e erase.me$$ && \
       diff ${ac_aux_dir}/config.sub erase.me$$ >/dev/null 2>&5
    then
      AC_MSG_RESULT(it works)
    else
      AC_MSG_RESULT(I'm deluding myself)
      LN_S=cp
    fi
    /bin/rm erase.me$$
  else
    LN_S=cp
  fi
])
