AC_DEFUN([CHECK_SSL],
[AC_MSG_CHECKING(for openssl)
SSLDIR=
found_ssl="no"
AC_ARG_WITH(ssl,
    AC_HELP_STRING([--with-ssl],
       [Use openssl (in specified installation directory)]),
    [check_ssl_dir="$withval"],
    [check_ssl_dir=])
for dir in $check_ssl_dir /usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr; do
   ssldir="$dir"
   if test -f "$dir/include/openssl/ssl.h"; then
     found_ssl="yes";
     SSLDIR="${ssldir}"
     CFLAGS="$CFLAGS -I$ssldir/include -I$ssldir/include/openssl";
     CXXFLAGS="$CXXFLAGS -I$ssldir/include -I$ssldir/include/openssl";
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
AC_MSG_RESULT($found_ssl)
if test x_$found_ssl != x_yes; then
   AC_MSG_ERROR([
----------------------------------------------------------------------
  Cannot find openssl libraries.

  Please install openssl or specify installation directory with
  --with-ssl=(dir).
----------------------------------------------------------------------
])
else
        printf "OpenSSL found in $ssldir\n";
        LIBS="$LIBS -lssl -lcrypto";
        LDFLAGS="$LDFLAGS -L$ssldir/lib";
	AC_DEFINE_UNQUOTED([USE_OPENSSL],[1],
	  ["Define to 1 if you want to use the openssl crypto library"])
	AC_SUBST(SSLDIR)
fi
])dnl
