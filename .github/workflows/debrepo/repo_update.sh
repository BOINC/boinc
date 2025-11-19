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

# support functions
function exit_on_fail() {
	errcode=$?
	errmsg=$1
	if [[ ! "$errcode" -eq "0" ]]; then
		printf "Failed command, exitcode: %d, %s\n" "$errcode" "$errmsg"
		exit 1
	fi
}

function exit_usage() {
	printf "Fail: $1\n"
	printf "Usage: repo_update.sh <allow-create> <repo-url> <incoming-dir> osversion release-type [release-key]\n"
	exit 1
}

CWD=$(pwd)
RELEASEKEY=boinc.gpg

# commandline params
ALLOW_CREATE=$1

BASEREPO=$2

SRC=$3
if [[ "$SRC" == "" ]]; then
	exit_usage "No base directory specified"
fi

DISTRO=$4

if [[ "$4" == "" ]]; then
	exit_usage "No OS version specified"
fi

if [[ ! "$5" == "" ]]; then
	case "$5" in
	"stable") TYPE="stable"
			  ;;
	"alpha") TYPE="alpha"
			  ;;
	"nightly") TYPE="nightly"
			  ;;
	"*")  exit_usage "Unrecognized repo type specified: $5"
			  ;;
	esac
fi

if [[ ! "$6" == "" ]]; then
	RELEASEKEY="$6"
fi

# static params
PUBKEYFILE=${SRC}/boinc.pub.key
PRIVKEYFILE=${SRC}/boinc.priv.key
KEYRING=${SRC}/trustedkeys.gpg

REPO="$BASEREPO/$TYPE/$DISTRO"
CONF_FILE="$CWD/aptly.$DISTRO.conf"

IS_MIRROR=1

# required files check
stat "$SRC" > /dev/null
exit_on_fail "No source directory present"

stat "$PUBKEYFILE" > /dev/null
exit_on_fail "No public key file present"

stat "$PRIVKEYFILE" > /dev/null
exit_on_fail "No private key file present"

pushd $CWD

rm -rf $CWD/http-data/$DISTRO/*

gpg2 --version

# import public key to allow the repo mirroring
gpg2 --no-default-keyring --primary-keyring $KEYRING --import $PUBKEYFILE || true
gpg2 --no-default-keyring --primary-keyring $KEYRING --import $PRIVKEYFILE || true
gpg2 --no-default-keyring --primary-keyring $KEYRING --list-keys

# create repo for indicated type and distribution
aptly -config=$CONF_FILE -distribution=$DISTRO repo create boinc-$TYPE
exit_on_fail "Could not create repository"

# mirror the currently deployed repo (if any)
aptly -config=$CONF_FILE -keyring=$KEYRING mirror create boinc-$TYPE-mirror $REPO $DISTRO
if [[ "$?" -eq "0" ]]; then
	# the command was successful and the mirror is created
	IS_MIRROR=0
fi

if [[ "$TYPE" == "stable" ]]; then
	# mirror alpha repo
	aptly -config=$CONF_FILE -keyring=$KEYRING mirror create boinc-alpha-mirror $BASEREPO/alpha/$DISTRO $DISTRO
	exit_on_fail "Could not mirror alpha repository"
	# update the packages from remote
	aptly -config=$CONF_FILE -keyring=$KEYRING mirror update boinc-alpha-mirror
fi

echo
echo "Is mirror: $IS_MIRROR"
echo "Can create: $ALLOW_CREATE"
echo

if [[ ! "$IS_MIRROR" -eq "0" ]]; then
	if [[ ! "$ALLOW_CREATE" -eq "0" ]]; then
		# if neither the mirror was created, nor the new creation allowed, terminate
		exit_usage "Could not mirror ${REPO} and creation is not allowed"
	else
		echo "Remote mirror failed but creation of new repo is allowed"
	fi
fi

if [[ "$IS_MIRROR" -eq "0" ]]; then
	# updates the packages from remote
	aptly -config=$CONF_FILE -keyring=$KEYRING mirror update boinc-$TYPE-mirror
	exit_on_fail "Failed to update the local mirror"

	# imports the downloaded packages to the local mirror
	aptly -config=$CONF_FILE repo import boinc-$TYPE-mirror boinc-$TYPE "Name"
	exit_on_fail "Failed to import the remote mirror into local"

	# keep only 3 last versions of each package before adding new ones
	packets=$(aptly -config=$CONF_FILE repo search boinc-$TYPE | grep -o '[^[:space:]]*_\([[:digit:]]*\.\)\{2\}[[:digit:]]*-\([[:digit:]]*_\)[^[:space:]]*' | sort -t '_' -k 2 -V | uniq)
    prefixes=$(echo "$packets"| cut -d '_' -f 1 | sort -n | uniq)
    suffixes=$(echo "$packets"| cut -d '_' -f 3- | sort -V | uniq)
    for prefix in $prefixes; do
        echo $prefix
        for suffix in $suffixes; do
            echo "  $suffix"
            matched_packets=$(echo "$packets" | grep "^${prefix}_.*_${suffix}$")
            count=$(echo "$matched_packets" | wc -l)
            echo "    count: $count"
            exceed=$(expr $count - 3)
            echo "    exceed: $exceed"
            if (( exceed > 0 )); then
                echo "$matched_packets" | head -n $exceed | while IFS= read -r packet; do
                    echo "      Remove: $packet"
					aptly -config=$CONF_FILE repo remove boinc-$TYPE $packet
					exit_on_fail "Failed to remove the package"
                done
            fi
        done
    done

	# creates the snapshot of the old situation
	aptly -config=$CONF_FILE snapshot create old-boinc-$TYPE-snap from repo boinc-$TYPE
	exit_on_fail "Failed to create old snapshot of the local repo"

	# info about the snapshot
	aptly -config=$CONF_FILE snapshot show old-boinc-$TYPE-snap
fi

if [[ "$TYPE" == "stable" ]]; then
	# get only one latest packages of each type from the alpha repo
	packets=$(aptly -config=$CONF_FILE mirror search boinc-alpha-mirror | grep -o '[^[:space:]]*_\([[:digit:]]*\.\)\{2\}[[:digit:]]*-\([[:digit:]]*_\)[^[:space:]]*' | sort -t '_' -k 2 -V -r | uniq)
    prefixes=$(echo "$packets"| cut -d '_' -f 1 | sort -n | uniq)
    suffixes=$(echo "$packets"| cut -d '_' -f 3- | sort -V | uniq)
    for prefix in $prefixes; do
        echo $prefix
        for suffix in $suffixes; do
            echo "  $suffix"
            matched_packets=$(echo "$packets" | grep "^${prefix}_.*_${suffix}$")
            echo "$matched_packets" | tail -n 1 | while IFS= read -r packet; do
                echo "      Adding: $packet"
                aptly -config=$CONF_FILE repo remove boinc-$TYPE $packet
                exit_on_fail "Failed to remove the package"
                done
        done
    done
else
	# imports into the repo the new packages
	aptly -config=$CONF_FILE repo add boinc-$TYPE $SRC/*.deb
	exit_on_fail "Failed to add new packages"
fi

if [[ "$IS_MIRROR" -eq "0" ]]; then
	# create new snapshot of the repo for deployment (with mirror)
	aptly -config=$CONF_FILE snapshot create new-boinc-$TYPE-snap from repo boinc-$TYPE
	exit_on_fail "Failed to create new snapshot of the local repo"

	# shows the contents of the new snapshot and its difference to the old one
	aptly -config=$CONF_FILE snapshot show new-boinc-$TYPE-snap
	aptly -config=$CONF_FILE snapshot diff new-boinc-$TYPE-snap old-boinc-$TYPE-snap

	# merges the two snapshots to allow offering old versions of packages
	aptly -config=$CONF_FILE snapshot merge -no-remove=true -latest=false boinc-$TYPE-snap new-boinc-$TYPE-snap old-boinc-$TYPE-snap
	exit_on_fail "Failed to merge old and new snapshots of the local repo"
else
	# create new snapshot of the repo for deployment (non-mirror)
	aptly -config=$CONF_FILE snapshot create boinc-$TYPE-snap from repo boinc-$TYPE
	exit_on_fail "Failed to create snapshot of the local repo"
fi

# publishes (to local dir) the updated snapshot
aptly -config=$CONF_FILE -keyring="$KEYRING" publish snapshot --batch=true boinc-$TYPE-snap
exit_on_fail "Failed to publish the snapshot of the local repo"

cd $CWD/http-data/$DISTRO/

# copy the key for the repo to the root of it
SRCKEYFILE="$SRC/$RELEASEKEY"
DSTKEYFILE="$CWD/http-data/$DISTRO/public/$RELEASEKEY"
cp "${SRCKEYFILE}" "${DSTKEYFILE}"
exit_on_fail "Failed to publish the public key to the repo"

find .

# Archive the produced repo to the archive in the format expected by the upload script:
# repo-<stable/alpha>-<osversion>.tar.gz
tar -zcvf ${SRC}/repo-$TYPE-$DISTRO.tar.gz -C public/ .
exit_on_fail "Could not package the repository for upload"

popd
