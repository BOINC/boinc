dnl
dnl MON_PKG_JAVA
dnl
dnl Check for a Java SDK
dnl
AC_DEFUN([MON_PKG_JAVA], [
	AC_ARG_ENABLE([java], AS_HELP_STRING([--enable-java],
		[Enable Java support @<:@auto@:>@]),
		[enable_java=$enableval],
		[enable_java=auto])

	AC_ARG_WITH([java_includes], AS_HELP_STRING([--with-java-includes=DIR],
		[Where to find jni.h]),
		[JAVA_INC=-I$withval],
		[JAVA_INC=])

	no_java=

	# Check the value of --enable-java
	if test "$enable_shared" = no; then
		# The JNI module cannot be built if there is no shared lib support
		AC_MSG_NOTICE([JNI support requires dynamic linking])
		no_java=yes
	fi
	if test "$enable_java" = no; then
		no_java=yes
	fi

	# Look for Java programs
	if test -z "$no_java"; then
		save_PATH="$PATH"
		if test "$JAVA_HOME" != ""; then
			PATH="$JAVA_HOME/bin:$PATH"
			export PATH
		fi

		# The java command is not needed for the build but is needed
		# to run the examples
		AC_PATH_PROG([JAVA], [java], [none])

		AC_PATH_PROG([JAVAC], [javac], [none])
		AC_PATH_PROG([JAVAH], [javah], [none])
		AC_PATH_PROG([JAR], [jar], [none])
		AC_PATH_PROG([JAVADOC], [javadoc], [none])

		PATH="$save_PATH"
		export PATH

		if test "$JAVAC" = none || test "$JAVAH" = none || \
				test "$JAR" = none || test "$JAVADOC" = none; then
			no_java=yes
		fi
	fi

	# Look for <jni.h>
	if test -z "$no_java"; then
		if test -z "$JAVA_INC"; then
			j=`echo $JAVAC | sed -e 's!/bin.*$!!'` #'
			JAVA_INC="-I$j/include"
		fi

		# Check for the arch-specific subdirectory containing <jni_md.h>
		JAVA_INC_ARCH=`echo $JAVA_INC | cut -c 3-`
		for i in "$JAVA_INC_ARCH"/*; do
			test -f "$i"/jni_md.h && JAVA_INC="$JAVA_INC -I$i"
		done

		save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $JAVA_INC"
		AC_CHECK_HEADER([jni.h],, [no_java=yes])
		CPPFLAGS="$save_CPPFLAGS"
	fi

	if test -z "$no_java"; then
		AC_CACHE_CHECK([for javah version], [mon_cv_javah_version],
			[mon_cv_javah_version=`$JAVAH -version | cut -d'"' -f2`])
		AH_TEMPLATE([HAVE_JDK_15], [Define if using JDK 1.5.x])
		case "$mon_cv_javah_version" in
			1.[[56789]].*)
				AC_DEFINE([HAVE_JDK_15])
				;;
		esac
	fi

	if test "$enable_java" = yes && test "$no_java" = yes; then
		AC_MSG_ERROR([Java development tools were not found])
	fi

	if test -n "$no_java"; then
		JAVA_INC=
		AC_MSG_NOTICE([Java support is disabled])
	fi

	AC_SUBST([JAVA_INC])
	AM_CONDITIONAL([HAVE_JAVA], [test -z "$no_java"])
])
