#!/bin/bash

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
	printf "Usage: repo_update.sh <allow-create> <repo-url> <incoming-dir> [osversion(jammy,focal,buster,bullseye)] [release-type(stable,alpha,nightly)] [release-key]\n"
	exit 1
}

CWD=$(pwd)
TYPE=stable
DISTRO=jammy
RELEASEKEY=boinc.gpg

# commandline params
ALLOW_CREATE=$1

BASEREPO=$2

SRC=$3
if [[ "$SRC" == "" ]]; then
	exit_usage "No base directory specified"
fi

if [[ ! "$4" == "" ]]; then
	DISTRO="$4"
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

gpg1 --version

# import public key to allow the repo mirroring
gpg1 --no-default-keyring --primary-keyring $KEYRING --import $PUBKEYFILE || true
gpg1 --no-default-keyring --primary-keyring $KEYRING --import $PRIVKEYFILE || true
gpg1 --no-default-keyring --primary-keyring $KEYRING --list-keys

# create repo for indicated type and distribution
aptly -config=$CONF_FILE -distribution=$DISTRO repo create boinc-$TYPE
exit_on_fail "Could not create repository"

# mirror the currently deployed repo (if any)
aptly -config=$CONF_FILE -keyring=$KEYRING mirror create boinc-$TYPE-mirror $REPO $DISTRO
if [[ "$?" -eq "0" ]]; then
	# the command was successful and the mirror is created
	IS_MIRROR=0
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
	# updates the the packages from remote
	aptly -config=$CONF_FILE -keyring=$KEYRING mirror update boinc-$TYPE-mirror
	exit_on_fail "Failed to update the local mirror"

	# imports the downloaded packages to the local mirror
	aptly -config=$CONF_FILE repo import boinc-$TYPE-mirror boinc-$TYPE "Name"
	exit_on_fail "Failed to import the remote mirror into local"

	# keep only 3 last versions of each package before adding new ones
	packets=$(aptly -config=$CONF_FILE repo search boinc-$TYPE | grep -o '[^[:space:]]*_\([[:digit:]]*\.\)\{2\}[[:digit:]]*-\([[:digit:]]*_\)[^[:space:]]*' | sort -t '_' -k 2 -V | uniq)
	declare -A split_lists
	packets_list=()
	while IFS= read -r line; do
		packets_list+=("$line")
	done <<< "$packets"
	for item in "${packets_list[@]}"; do
		prefix="${item%%_*}"     # Extract the prefix (text before the first underscore)
		split_lists["$prefix"]+="$item"$'\n'  # Append the item to the corresponding prefix's list
	done

	for prefix in "${!split_lists[@]}"; do
		echo "List for prefix: $prefix"
		echo "${split_lists[$prefix]}"
		count=$(echo "${split_lists[$prefix]}" | wc -l)
		number=$(expr $count - 1)
		echo "count=$number"
		i=0
		exceed=$(expr $number - 3)
		echo "exceed=$exceed"
		if (( exceed > 0)); then
			values_list=()
			while IFS= read -r line; do
				values_list+=("$line")
			done <<< "${split_lists[$prefix]}"
			for value in "${values_list[@]}"; do
				if (( i < exceed )); then
					echo "Remove: $value"
					i=$((i+1))
					aptly -config=$CONF_FILE repo remove boinc-$TYPE $value
					exit_on_fail "Failed to remove the package"
				else
					break
				fi
			done
		fi
	done

	# creates the snapshot of the old situation
	aptly -config=$CONF_FILE snapshot create old-boinc-$TYPE-snap from repo boinc-$TYPE
	exit_on_fail "Failed to create old snapshot of the local repo"

	# info about the snapshot
	aptly -config=$CONF_FILE snapshot show old-boinc-$TYPE-snap
fi

# imports into the repo the new packages
aptly -config=$CONF_FILE repo add boinc-$TYPE $SRC/*.deb
exit_on_fail "Failed to add new packages"

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
