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

# Script to run unit tests and report test coverage to codecov.io
# intended to be run inside a CI container

# check working directory because the script needs to be called like: ./3rdParty/buildCoverageReport.sh
if [ ! -d "3rdParty" ]; then
    echo "start this script in the source root directory"
    exit 1
fi
if [ ! -f "Makefile" ]; then
    echo "please run ./configure --enable-coverage first"
    exit 1
fi

make test

cd lib && gcov -l *.o && cd ..
cd sched && gcov -l *.o && cd ..

# disabled because codecov.io is not connected to github.com/BOINC/boinc
#bash <(curl -s https://codecov.io/bash) -X gcov -X coveragepy -s lib/ -s sched/
