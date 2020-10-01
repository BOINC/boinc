#! /usr/bin/env bash

# Description:
#   A wrapper around pylint that allows the user to specify whether it should
#   be executed with version 2 or 3 of the Python interpreter.
#
# Use:
#   pylint.sh [PYTHON_VERSION] PYLINT_ARGS
#
#   PYTHON_VERSION: Either 2 or 3, for version 2/3 of the Python interpreter.
#   PYLINT_ARGS: Argument to pass on to pylint: flags, module paths, etc.

python_interpreter="python${1}"
pylint_args="-f colorized ${@:2}"
pylint_import=$(cat << PYTHON
import sys
import pkg_resources
__requires__ = "pylint"
sys.exit(
	pkg_resources.load_entry_point("pylint", "console_scripts", "pylint")()
)
PYTHON
)

$python_interpreter -c "$pylint_import" $pylint_args

