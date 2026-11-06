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
#     CUDA_CCBIN
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
    AC_REQUIRE([AC_CANONICAL_HOST])

    AC_ARG_WITH([cuda],
        AS_HELP_STRING(
            [--with-cuda=PATH],
            [use CUDA toolkit installed in PATH]
        ),
        [cuda_prefix="$withval"],
        [cuda_prefix="/usr/local/cuda"]
    )

    AC_ARG_VAR([CUDA_NVCC], [CUDA compiler])
    AC_ARG_VAR([CUDA_CCBIN], [Host compiler for CUDA nvcc -ccbin])

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

        case "$host_cpu" in
            aarch64|arm64)
                if test -d "$cuda_root/targets/sbsa-linux"; then
                    cuda_target_dir="$cuda_root/targets/sbsa-linux"
                elif test -d "$cuda_root/targets/aarch64-linux"; then
                    cuda_target_dir="$cuda_root/targets/aarch64-linux"
                else
                    cuda_target_dir="$cuda_root"
                fi
                ;;
            *)
                cuda_target_dir="$cuda_root"
                ;;
        esac

        if test -d "$cuda_target_dir/lib64"; then
            cuda_lib_dir="$cuda_target_dir/lib64"
        else
            cuda_lib_dir="$cuda_target_dir/lib"
        fi

        CUDA_CPPFLAGS="-I$cuda_target_dir/include"
        CUDA_CFLAGS="--cuda-path=$cuda_root"
        CUDA_LIBS="-L$cuda_lib_dir -lcudart"


        AC_MSG_CHECKING([CUDA version])

        CUDA_VERSION=`$CUDA_NVCC --version 2>/dev/null |
            sed -n 's/.*release \([[0-9.]]*\),.*/\1/p'`

        AC_MSG_RESULT([$CUDA_VERSION])


        save_CPPFLAGS="$CPPFLAGS"
        save_LIBS="$LIBS"
        CPPFLAGS="$CPPFLAGS $CUDA_CPPFLAGS"
        LIBS="$LIBS $CUDA_LIBS"

        AC_CHECK_HEADER(
            [cuda_runtime.h],
            [
                AC_CHECK_LIB(
                    [cudart],
                    [cudaMalloc],
                    [cuda_found=yes],
                    [cuda_found=no]
                )
            ],
            [cuda_found=no]
        )

        CPPFLAGS="$save_CPPFLAGS"
        LIBS="$save_LIBS"


        AS_IF([test "x$cuda_found" = "xyes"], [
            if test -z "$CUDA_CCBIN"; then
                CUDA_CCBIN="$CXX"
            fi
            AC_MSG_CHECKING([whether $CUDA_NVCC works with -ccbin $CUDA_CCBIN])
            cat <<_ACEOF >conftest.cu
int main() { return 0; }
_ACEOF
            if $CUDA_NVCC -ccbin "$CUDA_CCBIN" -c conftest.cu -o conftest.o >&AS_MESSAGE_LOG_FD 2>&1; then
                AC_MSG_RESULT([yes])
            else
                AC_MSG_RESULT([no])
                cuda_found=no
            fi
            rm -f conftest.cu conftest.o
        ])


        AS_IF([test "x$cuda_found" = "xyes"], [
            AC_DEFINE(
                [HAVE_CUDA],
                [1],
                [Define if CUDA is available]
            )

            AC_SUBST([CUDA_NVCC])
            AC_SUBST([CUDA_CCBIN])
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
