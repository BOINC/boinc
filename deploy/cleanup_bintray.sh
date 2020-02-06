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

## Clean up old files and versions on Bintray (https://bintray.com/docs/api/)
## Parameters:
## --weekly_threshold textual representation of the time to keep the weekly builds (default: 6 months, used as argument to date -d)
## --tmp_dir directory where API responses are stored (default: /tmp)

# be carefull with set -x in this script because of ${BINTRAY_API_KEY} which needs to stay secret
set -e # Exit on errors
trap 'exit 1' ERR

# check working directory because the script needs to be called like: ./deploy/cleanup_bintray.sh
if [ ! -d "deploy" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
TMPDIR="/tmp"
weekly_threshold="6 months"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --tmp_dir)
        TMPDIR="$2"
        shift
        ;;
        --weekly_threshold)
        weekly_threshold="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

command -v jq >/dev/null 2>&1 || { echo >&2 "jq (command-line json parser) is needed but not installed.  Aborting."; exit 1; }

# this needs to be set in the environment before starting the script
if [ "${BINTRAY_API_KEY}" == "" ] ; then
    echo "BINTRAY_API_KEY is missing; doing nothing"
    exit 0
fi

BINTRAY_API=https://api.bintray.com
BINTRAY_USER="${BINTRAY_USER:-ChristianBeer}"
BINTRAY_API_KEY="$BINTRAY_API_KEY" # env
BINTRAY_REPO="${BINTRAY_REPO:-boinc-ci}"
BINTRAY_REPO_OWNER="${BINTRAY_REPO_OWNER:-boinc}" # owner and user not always the same

GITHUB_API=https://api.github.com
GITHUB_USER="${GITHUB_USER:-BOINC}"
# github has a rate limit of 60 requests per hour for non authenticated requests
# see https://developer.github.com/v3/#authentication on how to authenticate
#GITHUB_OAUTH="-H \"Authorization: token OAUTH-TOKEN\""
GITHUB_OAUTH=""
GITHUB_REPO="${GITHUB_REPO:-boinc}"

BINTRAY_CURL="curl -s -S -u${BINTRAY_USER}:${BINTRAY_API_KEY} -H Accept:application/json -w \n"
GITHUB_CURL="curl -s -S ${GITHUB_OAUTH} -H Accept:application/vnd.github.v3+json -w \n"

# get quotas from bintray
${BINTRAY_CURL} "${BINTRAY_API}/orgs/${BINTRAY_REPO_OWNER}" -o "${TMPDIR}/organisation.txt"

quota_used=$(jq '.quota_used_bytes' < "${TMPDIR}/organisation.txt")
free_storage=$(jq '.free_storage' < "${TMPDIR}/organisation.txt")
free_storage_quota_limit=$(jq '.free_storage_quota_limit' < "${TMPDIR}/organisation.txt")
storage_percent=$(echo "scale=2; $quota_used / $free_storage_quota_limit *100" | bc)
echo "Storage quota used: ${storage_percent}%"

last_month_download=$(jq '.last_month_free_downloads' < "${TMPDIR}/organisation.txt")
monthly_download_quota=$(jq '.monthly_free_downloads_quota_limit' < "${TMPDIR}/organisation.txt")
download_percent=$(echo "scale=2; $last_month_download / $monthly_download_quota *100" | bc)
echo "Download quota used: ${download_percent}%"

rm -f "${TMPDIR}/organisation.txt"

BINTRAY_PACKAGE="pull-requests"
echo "Cleaning package: $BINTRAY_PACKAGE"

${BINTRAY_CURL} "${BINTRAY_API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${BINTRAY_PACKAGE}" -o "${TMPDIR}/package_pr.txt"

pr_ids=$(jq -r .versions[]? < "${TMPDIR}/package_pr.txt" | cut -d_ -f1 | cut -b 3- | sort -u)
for pr in $pr_ids
do
    echo "$pr"
    state=$(${GITHUB_CURL} "${GITHUB_API}/repos/${GITHUB_USER}/${GITHUB_REPO}/pulls/${pr}" | jq -r .state)
    if [ "$state" = "closed" ]; then
        echo "  is closed"
        data="[{\"PR\": [\"${pr}\"]}]"
        ${BINTRAY_CURL} -H Content-Type:application/json -X POST -d "${data}" "${BINTRAY_API}/search/attributes/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${BINTRAY_PACKAGE}/versions" -o "${TMPDIR}/versions.txt"
        versions=$(jq -r .[].name < "${TMPDIR}/versions.txt")
        for version in $versions
        do
            echo "Deleting ${version}"
            ${BINTRAY_CURL} -X DELETE "${BINTRAY_API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${BINTRAY_PACKAGE}/versions/${version}"
        done
    fi
done

rm -f "${TMPDIR}/package_pr.txt" "${TMPDIR}/versions.txt"
echo "Finished cleaning package: $BINTRAY_PACKAGE"

BINTRAY_PACKAGE="weekly"
echo "Cleaning package: $BINTRAY_PACKAGE"

time_threshold=$(date -u -d "now-${weekly_threshold}" +%s)
data="[{\"create_time_utc\": \"[,${time_threshold}]\"}]"

${BINTRAY_CURL} -H Content-Type:application/json -X POST -d "${data}" "${BINTRAY_API}/search/attributes/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${BINTRAY_PACKAGE}/versions?attribute_values=1" -o "${TMPDIR}/versions.txt"
versions=$(jq -r .[].name? < "${TMPDIR}/versions.txt")
for version in $versions
do
    echo "Deleting ${version}"
    ${BINTRAY_CURL} -X DELETE "${BINTRAY_API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${BINTRAY_PACKAGE}/versions/${version}"
done

rm -f "${TMPDIR}/versions.txt"
echo "Finished cleaning package: $BINTRAY_PACKAGE"
