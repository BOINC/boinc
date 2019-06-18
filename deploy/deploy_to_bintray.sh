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

## Push binaries to Bintray (https://bintray.com/docs/api/)
## Uploads a specific 7z archive from directory given by $1
## TODO: change arguments to include BOINC_TYPE and FILEPATH, eventually allow multiple files

# be carefull with set -x in this script because of ${BINTRAY_API_KEY} which needs to stay secret
set -e # Exit on errors
trap 'exit 1' ERR

# check working directory because the script needs to be called like: ./deploy/deploy_to_bintray.sh
if [ ! -d "deploy" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
if [[ $# != 1 ]]; then
    echo "Usage: $0 SOURCE_DIR"
    echo "SOURCE_DIR : relative path where binaries are, last component is used as BOINC_TYPE"
    exit 1
fi

SOURCE_DIR="$1"
if [[ ! -d "${SOURCE_DIR}" || $(ls -A "${SOURCE_DIR}" | wc -l) -eq 0 ]]; then
    echo "Directory '$SOURCE_DIR' doesn't exist or is empty";
    exit 1;
fi

# for PR's this is set if the PR comes from within the same repository
if [ "${BINTRAY_API_KEY}" == "" ] ; then
    echo "BINTRAY_API_KEY is missing; doing nothing"
    exit 0
fi

CI_RUN="${TRAVIS:-false}"
BOINC_TYPE="$(basename "${SOURCE_DIR}")" # TODO: do not infer TYPE from directory, instead make it an argument
API=https://api.bintray.com
BINTRAY_USER="${BINTRAY_USER:-ChristianBeer}"
BINTRAY_API_KEY="$BINTRAY_API_KEY" # env
BINTRAY_REPO="${BINTRAY_REPO:-boinc-ci}"
BINTRAY_REPO_OWNER="${BINTRAY_REPO_OWNER:-boinc}" # owner and user not always the same
WEBSITE_URL="${WEBSITE_URL:-https://boinc.berkeley.edu}"
ISSUE_TRACKER_URL="${ISSUE_TRACKER_URL:-https://github.com/BOINC/boinc/issues}"
VCS_URL="${VCS_URL:-https://github.com/BOINC/boinc.git}" # Mandatory for packages in free Bintray repos

CURL="curl -u${BINTRAY_USER}:${BINTRAY_API_KEY} -H Accept:application/json -w \n"
#CURL="echo " # use this for local debugging

BUILD_DATE=$(date "+%Y-%m-%d")
GIT_REV=$(git rev-parse --short HEAD)
PKG_NAME="custom"
PKG_DESC="Automated CI build of BOINC components"
VERSION="custom_${BUILD_DATE}_${GIT_REV}"
VERSION_DESC="Custom build created on ${BUILD_DATE}"
RUN_CLEANUP="false"
if [[ $CI_RUN == "true" ]]; then
    case $TRAVIS_EVENT_TYPE in
        pull_request)
            PKG_NAME="pull-requests"
            GIT_REV=${TRAVIS_PULL_REQUEST_SHA:0:8}
            VERSION="PR${TRAVIS_PULL_REQUEST}_${BUILD_DATE}_${GIT_REV}"
            VERSION_DESC="CI build created from PR #${TRAVIS_PULL_REQUEST} on ${BUILD_DATE}"
        ;;
        cron)
            PKG_NAME="weekly"
            VERSION="weekly_${BUILD_DATE}_${GIT_REV}"
            VERSION_DESC="Weekly CI build created on ${BUILD_DATE}"
            # run cleanup script once a month
            if [[ $(date +%d) -lt 8 ]]; then
                RUN_CLEANUP="true"
            fi
        ;;
        *)
            echo "event $TRAVIS_EVENT_TYPE not supported for deployment"
            exit 0;
        ;;
    esac
fi

echo "Creating package ${PKG_NAME}..."
data="{
    \"name\": \"${PKG_NAME}\",
    \"desc\": \"${PKG_DESC}\",
    \"desc_url\": \"auto\",
    \"website_url\": [\"${WEBSITE_URL}\"],
    \"vcs_url\": [\"${VCS_URL}\"],
    \"issue_tracker_url\": [\"${ISSUE_TRACKER_URL}\"],
    \"licenses\": [\"LGPL-3.0\"]
    }"
${CURL} -H Content-Type:application/json -X POST -d "${data}" "${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}"

echo "Creating version ${VERSION}"
data="{
    \"name\": \"${VERSION}\",
    \"desc\": \"${VERSION_DESC}\"
}"
set +x
${CURL} -H Content-Type:application/json -X POST -d "${data}" "${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PKG_NAME}/versions"

echo "Adding attributes to version ${VERSION}"
# this can be used to cleanup old versions
pr="${TRAVIS_PULL_REQUEST:-0}"
created=$(date -u +%s)
data="[{\"name\": \"PR\", \"values\": [\"${pr}\"], \"type\": \"string\"},
            {\"name\": \"create_time_utc\", \"values\": [${created}], \"type\": \"number\"}]"
echo $data
${CURL} -H Content-Type:application/json -X POST -d "${data}" "${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PKG_NAME}/versions/${VERSION}/attributes"

echo "Uploading and publishing ${SOURCE_DIR}/${BOINC_TYPE}.7z..."
if [ -f "${SOURCE_DIR}/${BOINC_TYPE}.7z" ]; then
    set +x
    ${CURL} -H Content-Type:application/octet-stream -T "${SOURCE_DIR}/${BOINC_TYPE}.7z" "${API}/content/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PKG_NAME}/${VERSION}/${BOINC_TYPE}_${VERSION}.7z?publish=1&override=1"
fi

if [ "$TRAVIS_BUILD_ID" ] ; then
    echo "Adding Travis CI log to release notes..."
    BUILD_LOG="https://travis-ci.org/BOINC/boinc/builds/${TRAVIS_BUILD_ID}"
    #BUILD_LOG="https://api.travis-ci.org/jobs/${TRAVIS_JOB_ID}/log.txt?deansi=true"
    data='{
    "bintray": {
    "syntax": "markdown",
    "content": "'${BUILD_LOG}'"
  }
}'
    set +x
    ${CURL} -H Content-Type:application/json -X POST -d "${data}" "${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PKG_NAME}/versions/${VERSION}/release_notes"
fi

if [[ $RUN_CLEANUP == "true" ]]; then
    ./deploy/cleanup_bintray.sh
fi
