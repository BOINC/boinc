AC_DEFUN([CHECK_SSL],
[
SSLDIR=
SSL_LIBS=
SSL_CFLAGS=
SSL_CXXFLAGS=
AC_ARG_WITH(pkg-config,
    AC_HELP_STRING([--with-pkg-config],
       [Use pkg-config specified]),
    [mypkgconfig="${withval}"]
    [mypkgconfig=])


AC_PATH_PROG(PKGCONFIG,[$mypkgconfig pkg-config])
AC_ARG_WITH(ssl,
    AC_HELP_STRING([--with-ssl],
       [Use openssl (in specified installation directory)]),
    [check_ssl_dir="$withval"],
    [check_ssl_dir=])
if test "x${PKGCONFIG}" != "x" -a "x${check_ssl_dir}" = "x" ; then
    SSLDIR="`${PKGCONFIG} openssl --cflags-only-I | sed -e 's/-I//' -e 's/\/include\/openssl//' -e 's/\/include//' | grep '[a-z]' 2>/dev/null`"
    if test "x${SSLDIR}" = "x" ; then
      if test -d "/usr/include/openssl" -o -f "/usr/include/ssl.h" ; then
        SSLDIR="/usr"
      fi
    fi
    if test "x${enable_client_release}" = "xyes" -o "x${disable_static_linkage}" = "xno" ; then
      SSL_LIBS="`${PKGCONFIG} openssl --libs --static 2>/dev/null`"
    fi  
    if test "x${SSL_LIBS}" = "x" ; then
      SSL_LIBS="`${PKGCONFIG} openssl --libs 2>/dev/null`"
    fi


    SSL_CFLAGS="`${PKGCONFIG} openssl --cflags 2>/dev/null`"
    SSL_CXXFLAGS="${SSL_CFLAGS}"
    SSL_CPPFLAGS="${SSL_CFLAGS}"
    found_ssl="yes"
fi
if test "x${SSL_LIBS}" = "x" ; then
  found_ssl="no"
  for dir in $check_ssl_dir ${prefix} /usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr /opt/misc /opt/csw /opt/sfw; do
    ssldir="$dir"
    if test -f "$dir/include/openssl/ssl.h"; then
      found_ssl="yes";
      SSLDIR="${ssldir}"
      SSL_CFLAGS="-I$ssldir/include -I$ssldir/include/openssl";
      SSL_CXXFLAGS="-I$ssldir/include -I$ssldir/include/openssl";
      SSL_LIBS="-L$ssldir -lssl -lcrypto -ldl -lz"
      break;
    fi
    if test -f "$dir/include/ssl.h"; then
      found_ssl="yes"
      SSLDIR="${ssldir}"
      SSL_CFLAGS="-I$ssldir/include/"
      SSL_CXXFLAGS="-I$ssldir/include/"
      SSL_LIBS="-L$ssldir -lssl -lcrypto"
      break;
    fi
  done
fi
AC_MSG_CHECKING(for openssl)
AC_MSG_RESULT($found_ssl)

BOINC_CHECK_LIB_WITH([gcrypt],[gcry_randomize],[SSL_LIBS])
BOINC_CHECK_LIB_WITH([gpg-error],[gpg_strerror], [SSL_LIBS])

if test x_$found_ssl != x_yes; then
   AC_MSG_ERROR([
----------------------------------------------------------------------
  Cannot find openssl libraries.

  Please install openssl or specify installation directory with
  --with-ssl=(dir).
----------------------------------------------------------------------
])
else
        printf "OpenSSL found in $SSLDIR\n";
	AC_DEFINE_UNQUOTED([USE_OPENSSL],[1],
	  ["Define to 1 if you want to use the openssl crypto library"])
	AC_SUBST(SSLDIR)
	AC_SUBST(SSL_CFLAGS)
	AC_SUBST(SSL_CXXFLAGS)
	AC_SUBST(SSL_LIBS)
fi
])
