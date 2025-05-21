#!/bin/bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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

# Script to execute googletest based unit tests for BOINC

# Usage:
#  ./tests/executeUnitTests.sh [--report-coverage] [--clean]
#
# if --report-coverage is given the test coverage will be reported to codecov.io
# if --clean is given the tests will be rebuild from scratch otherwise an existing
# build directory is used

# check working directory because the script needs to be called like: ./tests/executeUnitTests.sh
if [ ! -d "tests" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
report=""
doclean=""
xml=""

while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --report-coverage)
        report="yes"
        ;;
        --report-xml)
        xml="yes"
        ;;
        --clean)
        doclean="yes"
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

command -v cmake >/dev/null 2>&1 || { echo >&2 "cmake is needed but not installed.  Aborting."; exit 1; }

if [ "${report}" = "yes" ]; then
    command -v gcov >/dev/null 2>&1 || { echo >&2 "gcov (lcov) is needed but not installed.  Aborting."; exit 1; }
fi

cd tests || exit 1
if [[ "$doclean" = "yes" && -d "gtest" ]]; then
    rm -rf gtest
    if [ $? -ne 0 ]; then cd ..; exit 1; fi
fi
mkdir -p gtest
if [ $? -ne 0 ]; then cd ..; exit 1; fi
cd gtest
cmake ../unit-tests
if [ $? -ne 0 ]; then cd ../..; exit 1; fi
make
if [ $? -ne 0 ]; then cd ../..; exit 1; fi

if [[ "$OSTYPE" == "darwin"* ]]; then
    MODULES="lib"
else
    MODULES="lib sched"
fi
for T in ${MODULES}; do
    XML_FLAGS=""
    if [ "${xml}" = "yes" ]; then
        XML_FLAGS="--gtest_output=xml:${T}_xml_report.xml"
    fi
    [ -d "${T}" ] && ./${T}/test_${T} ${XML_FLAGS};
done
