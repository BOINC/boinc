# ===========================================================================
#   AX_CUDA
# ---------------------------------------------------------------------------
#   Detect NVIDIA CUDA toolkit.
#
#   Provides:
#     CUDA_CPPFLAGS
#     CUDA_CFLAGS
#     CUDA_LIBS
#     CUDA_NVCC
#     CUDA_VERSION
#
#   Usage:
#     AX_CUDA
#
#   Optional:
#     --with-cuda=PATH
#
# ===========================================================================

AC_DEFUN([AX_CUDA],
[
    AC_ARG_WITH([cuda],
        AS_HELP_STRING(
            [--with-cuda=PATH],
            [use CUDA toolkit installed in PATH]
        ),
        [cuda_prefix="$withval"],
        [cuda_prefix="/usr/local/cuda"]
    )

    AC_ARG_VAR([CUDA_NVCC], [CUDA compiler])

    cuda_found=no

    if test -n "$CUDA_NVCC"; then
        if test -x "$CUDA_NVCC"; then
            cuda_found=yes
        fi
    elif test -x "$cuda_prefix/bin/nvcc"; then
        CUDA_NVCC="$cuda_prefix/bin/nvcc"
        cuda_found=yes
    else
        AC_PATH_PROG([CUDA_NVCC], [nvcc])

        if test -n "$CUDA_NVCC"; then
            cuda_found=yes
        fi
    fi


    AS_IF([test "x$cuda_found" = "xyes"], [
        AC_MSG_NOTICE([checking CUDA support])

        cuda_root=`dirname "$CUDA_NVCC"`
        cuda_root=`cd "$cuda_root/.." && pwd`

        CUDA_CPPFLAGS="-I$cuda_root/include"
        CUDA_CFLAGS="--cuda-path=$cuda_root"
        CUDA_LIBS="-L$cuda_root/lib64 -lcudart"


        AC_MSG_CHECKING([CUDA version])

        CUDA_VERSION=`$CUDA_NVCC --version 2>/dev/null |
            sed -n 's/.*release \([[0-9.]]*\),.*/\1/p'`

        AC_MSG_RESULT([$CUDA_VERSION])


        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $CUDA_CPPFLAGS"

        AC_CHECK_HEADER(
            [cuda_runtime.h],
            [],
            [cuda_found=no]
        )

        CPPFLAGS="$save_CPPFLAGS"


        AS_IF([test "x$cuda_found" = "xyes"], [
            AC_DEFINE(
                [HAVE_CUDA],
                [1],
                [Define if CUDA is available]
            )

            AC_SUBST([CUDA_NVCC])
            AC_SUBST([CUDA_CPPFLAGS])
            AC_SUBST([CUDA_CFLAGS])
            AC_SUBST([CUDA_LIBS])
            AC_SUBST([CUDA_VERSION])

            AC_MSG_NOTICE([CUDA enabled])
        ], [
            AC_MSG_WARN([CUDA not usable, disabling CUDA support])
        ])

    ], [
        AC_MSG_NOTICE([CUDA not found, disabling CUDA support])
    ])
])
