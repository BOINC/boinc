AC_DEFUN([CHECK_SSL],
[AC_MSG_CHECKING(if ssl is wanted)
SSLDIR=
AC_ARG_WITH(ssl,
[  --with-ssl enable ssl [will check /usr/local/ssl
                            /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr ]
],
[   AC_MSG_RESULT(yes)
    for dir in $withval /usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr; do
        ssldir="$dir"
        if test -f "$dir/include/openssl/ssl.h"; then
            found_ssl="yes";
	    SSLDIR="${ssldir}"
            CFLAGS="$CFLAGS -I$ssldir/include/openssl";
            CXXFLAGS="$CXXFLAGS -I$ssldir/include/openssl";
            break;
        fi
        if test -f "$dir/include/ssl.h"; then
            found_ssl="yes";
	    SSLDIR="${ssldir}"
            CFLAGS="$CFLAGS -I$ssldir/include/";
            CXXFLAGS="$CXXFLAGS -I$ssldir/include/";
            break
        fi
    done
    if test x_$found_ssl != x_yes; then
        AC_MSG_ERROR(Cannot find ssl libraries)
    else
        printf "OpenSSL found in $ssldir\n";
        LIBS="$LIBS -lssl -lcrypto";
        LDFLAGS="$LDFLAGS -L$ssldir/lib";
	AC_DEFINE_UNQUOTED([USE_OPENSSL],[1],
	  ["Define to 1 if you want to use the openssl crypto library"])
	AC_SUBST(SSLDIR)
    fi
],
[
    AC_DEFINE_UNQUOTED([USE_RSAEURO],[1],
      ["Define to 1 if you want to use the RSAEURO crypto library"])
    AC_MSG_RESULT(no)
])
])dnl
