#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2020 University of California
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

# Script to execute pylint tests

# check working directory because the script needs to be called like: ./tests/executePylintTests.sh
if [ ! -d "tests" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

hasbitset () { (( $1 & 2**($2-1) )) && return 0 || return 1; }

ROOTDIR=$(pwd)

SRC=(
    'lib/submit_api.py'
    'py/Boinc/'
    'samples/vm_wrapper/setupPyBOINC.py'
    'samples/vm_wrapper/VMwrapper.py'
    'sched/assimilator.py'
    'sched/pymw_assimilator.py'
    'sched/start'
    'sched/testasm.py'
    'stripchart/samples/parse_config'
    'tools/appmgr'
    'tools/check_project'
    'tools/dbcheck_files_exist'
    'tools/make_project'
    'tools/parse_config'
    'tools/submit_api_test.py'
    'tools/update_versions_v6'
    'tools/upgrade'
)

${ROOTDIR}/tests/pylint.sh $1 ${ROOTDIR}/${SRC[@]}

EXITCODE=$?

(hasbitset ${EXITCODE} 1 || hasbitset ${EXITCODE} 2 || hasbitset ${EXITCODE} 6) && exit 1 || exit 0
