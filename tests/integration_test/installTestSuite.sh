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
## The testsuite will also be cloned into test_dir

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
BOINC_SERVER_DOCKER_DIR=/tmp/boinc-server-docker
INTEGRATION_LOG_DIR=${BOINC_INTEGRATION_LOG_DIR:-${TMPDIR:-/tmp}/boinc-integration-logs}
STARTUP_TIMEOUT_SECONDS=${BOINC_SERVER_STARTUP_TIMEOUT_SECONDS:-300}
STARTUP_POLL_INTERVAL_SECONDS=${BOINC_SERVER_STARTUP_POLL_INTERVAL_SECONDS:-5}
docker_logs_pid=""
test_dir=""

start_docker_log_stream() {
    current_dir=$(pwd)

    if [ ! -f "${BOINC_SERVER_DOCKER_DIR}/docker-compose.yml" ]; then
        return
    fi

    mkdir -p "${INTEGRATION_LOG_DIR}"
    : > "${INTEGRATION_LOG_DIR}/docker-compose-live.log"

    cd "${BOINC_SERVER_DOCKER_DIR}" || exit 1
    docker compose logs -f --no-color > "${INTEGRATION_LOG_DIR}/docker-compose-live.log" 2>&1 &
    docker_logs_pid=$!
    cd "${current_dir}" || exit 1
}

stop_docker_log_stream() {
    if [ -n "${docker_logs_pid}" ] && kill -0 "${docker_logs_pid}" 2>/dev/null; then
        kill "${docker_logs_pid}"
        wait "${docker_logs_pid}" 2>/dev/null
    fi
}

on_exit() {
    status=$1
    trap - EXIT

    stop_docker_log_stream

    if [ "${status}" -ne 0 ] && [ -f "${INTEGRATION_LOG_DIR}/docker-compose-live.log" ]; then
        echo
        echo "Docker compose logs (last 200 lines):"
        tail -n 200 "${INTEGRATION_LOG_DIR}/docker-compose-live.log"
        echo
        echo "Integration test diagnostics saved to ${INTEGRATION_LOG_DIR}"
    fi
}

trap 'on_exit $?' EXIT
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
composer require --dev phpunit/phpunit
if [ $? -ne 0 ]; then exit 1; fi
composer require --dev guzzlehttp/guzzle
if [ $? -ne 0 ]; then exit 1; fi
composer update --dev
if [ $? -ne 0 ]; then exit 1; fi
cd .. || exit 1

cd "${PREFIX}/manage" || exit 1
ansible-playbook -i hosts build.yml --extra-vars "boinc_dir=${ROOTDIR}"
if [ $? -ne 0 ]; then exit 1; fi

ansible-playbook -i hosts start.yml
if [ $? -ne 0 ]; then exit 1; fi

start_docker_log_stream

startup_deadline=$((SECONDS + STARTUP_TIMEOUT_SECONDS))
until curl -o /dev/null -SsifL http://127.0.0.1/boincserver/index.php; do
    if [ "${SECONDS}" -ge "${startup_deadline}" ]; then
        echo
        echo "Timed out waiting ${STARTUP_TIMEOUT_SECONDS} seconds for http://127.0.0.1/boincserver/index.php"
        mkdir -p "${INTEGRATION_LOG_DIR}"
        curl -v http://127.0.0.1/boincserver/index.php > "${INTEGRATION_LOG_DIR}/curl-startup-check.txt" 2>&1
        exit 1
    fi
    printf '.'
    sleep "${STARTUP_POLL_INTERVAL_SECONDS}"
done
printf '\n'

cd "${ROOTDIR}" || exit 1
