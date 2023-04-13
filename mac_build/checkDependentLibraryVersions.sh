#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2023 University of California
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
#
# Script to check that dependent library versions expected by Xcode project
# match those specified in mac_build/dependencyNames.sh.

source "${PROJECT_DIR}/dependencyNames.sh"

errorFound=0

function compareVersion {
    ## BuildMacBOINC-CI.sh adds a path to a directory named freetype2
    ## We treat this as a special case so as not to give a false
    ## positive with freetypeBaseName and freetypeDirName
    theSearchPath=`echo "${1}" | sed s/"freetype2"/"free-type2"/g`
    for (( i=0; i<${#baseNames[@]}; i++ )); do
        if [[ "${theSearchPath}" == *"${baseNames[$i]}"* ]]; then
            ## This search path contans this dependent library name
            if [[ "${theSearchPath}" != *"${dirNames[$i]}"* ]]; then
                errorFound=1
                s1=${1#*${baseNames[$i]}}
                s2=${s1%%/*}
                ## Output error to stderr to avoid suppression by xcpretty
                echo "ERROR: Xcode project $2 has "${baseNames[$i]}${s2}" but dependencyNames.sh has "${dirNames[$i]} "(Building $3)" | tee -a /tmp/depversions.txt
            fi
        fi
    done
}

##echo "checking HEADER_SEARCH_PATHS"
compareVersion "${HEADER_SEARCH_PATHS}" "HEADER_SEARCH_PATHS" "${TARGET_NAME}"

##echo "checking USER_HEADER_SEARCH_PATHS"
compareVersion "${USER_HEADER_SEARCH_PATHS}" "USER_HEADER_SEARCH_PATHS" "${TARGET_NAME}"

##echo "checking LIBRARY_SEARCH_PATHS"
compareVersion "${LIBRARY_SEARCH_PATHS}" "LIBRARY_SEARCH_PATHS" "${TARGET_NAME}"

return $errorFound
