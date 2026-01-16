#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2018 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

## support script to install the boinc server docker environment
## test_dir must be outside of the code directory because the code is copied/symlinked
## into the testsuite

# checks if a given path is canonical (absolute and does not contain relative links)
# from http://unix.stackexchange.com/a/256437
isPathCanonical() {
  case "x$1" in
    (x*/..|x*/../*|x../*|x*/.|x*/./*|x./*)
        rc=1
        ;;
    (x/*)
        rc=0
        ;;
    (*)
        rc=1
        ;;
  esac
  return $rc
}

# checks if first argument is a subpath of second argument
isPathSubpath() {
  case $(readlink -f $1)/ in
    $(readlink -f $2)/*)
      rc=0
      ;;
    $(readlink -f $1))
      rc=0
      ;;
    *)
      rc=1
      ;;
  esac
  return $rc
}

# check working directory because the script needs to be called like: ./integration_test/installTestSuite.sh
if [ ! -d "tests/integration_test" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
PREFIX=$ROOTDIR/tests/server-test
test_dir=""
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --test_dir)
        test_dir="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ "x$test_dir" != "x" ]; then
    if isPathCanonical "$test_dir" && [ "$test_dir" != "/" ]; then
      if isPathSubpath "$test_dir" "$ROOTDIR"; then
          echo "test_dir must not be a subdirectory of $ROOTDIR"
          exit 1
      else
          PREFIX="$test_dir"
      fi
    else
        echo "test_dir must be an absolute path without ./ or ../ in it"
        exit 1
    fi
fi

cd "${PREFIX}/tests" || exit 1

composer test
if [ $? -ne 0 ]; then exit 1; fi

cd "${ROOTDIR}" || exit 1
