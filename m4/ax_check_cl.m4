
# OpenCL support
AC_DEFUN([AX_CHECK_CL],
    [AC_ARG_WITH([opencl],
        [AS_HELP_STRING([--with-opencl],
            [build OpenClApp with OpenCL @<:@default=check@:>@])],
        [],
        [with_opencl=check])

    # detect/verify we have a working OpenCL header and library
    AS_IF([test "x$with_opencl" != xno],
        [
            # check for usable OpenCL header
            # note C++ binding uses opencl.h, not just cl.h
            AC_MSG_NOTICE([checking for usable OpenCL opencl.h header])
            AC_CHECK_HEADER([OpenCL/opencl.h],
                [OPENCL_HEADER='"OpenCL/opencl.h"'],
                [AC_CHECK_HEADER([CL/opencl.h],
                    [OPENCL_HEADER='"CL/opencl.h"'],
                    [if test "x$with_opencl" != xcheck; then
                        AC_MSG_FAILURE([OpenCL support was requested, but no usable OpenCL header was found])
                    fi
                    ]
                )
                ]
            )
            AS_IF([test "x$OPENCL_HEADER" != x],
                [
                # we have a header, check for usable OpenCL library
                # we cannot use AC_CHECK_LIB because we want to check
                # using the OS X OpenCL framework, and AC_CHECK_LIB does not
                # know how to use the framework linker syntax.
                # (we also do not want the OpenCL libs to be added to the
                # LIBS variable, and AC_CHECK_LIB does that if it succeeds)
                AC_MSG_CHECKING([for usable OpenCL library])
                savedLIBS=$LIBS
                OPENCL_LIBS="-framework OpenCL"
                LIBS="$OPENCL_LIBS $savedLIBS"
                AC_LINK_IFELSE( [AC_LANG_PROGRAM([@%:@include $OPENCL_HEADER],[clGetPlatformIDs(0,0,0);])],
                    [AC_MSG_RESULT($OPENCL_LIBS)],
                    [OPENCL_LIBS="-lOpenCL"
                    LIBS="$OPENCL_LIBS $savedLIBS"
                    AC_LINK_IFELSE( [AC_LANG_PROGRAM([@%:@include $OPENCL_HEADER],[clGetPlatformIDs(0,0,0);])],
                        [AC_MSG_RESULT($OPENCL_LIBS)],
                        [AC_MSG_RESULT([no])
                        if test "x$with_opencl" != xcheck; then
                            AC_MSG_FAILURE([OpenCL support was requested, but no usable OpenCL library was found])
                        fi
                        ]
                    )
                    ]
                )
                LIBS=$savedLIBS
                ]
            )
            AS_IF([test "x$OPENCL_HEADER" != "x" -a "x$OPENCL_LIBS" != "x"],
                [with_opencl=yes
                MPI_SUBDIRS="$MPI_SUBDIRS opencl"
                AC_SUBST([OPENCL_LIBS])
                ],
                [AC_MSG_NOTICE([no usable OpenCL installation found])
                with_opencl=no
                ])
        ])
])dnl
