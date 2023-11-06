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
	printf "Usage: repo_update.sh <allow-create> <repo-url> <incoming-dir> [osversion] [release-type] [release-key] [key-hash] [arch]\n"
	exit 1
}

CWD=$(pwd)
TYPE=stable
DISTRO=fc38
ARCH=x86_64
RELEASEKEY=boinc.gpg

# commandline params
ALLOW_CREATE=$1

BASEREPO=$2

SRC=$3
if [[ "$SRC" == "" ]]; then
	exit_usage "No base directory specified"
fi

DISTRO="$4"

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

RELEASEKEY="$6"
HASH="$7"
ARCH="$8"

# static params
PUBKEYFILE=${SRC}/boinc.pub.key
PRIVKEYFILE=${SRC}/boinc.priv.key

RPMSRC="$SRC"

IS_MIRROR=1

# required files check
stat "$RPMSRC" > /dev/null
exit_on_fail "No source directory present"

stat "$PUBKEYFILE" > /dev/null
exit_on_fail "No public key file present"

stat "$PRIVKEYFILE" > /dev/null
exit_on_fail "No private key file present"

pushd $CWD

gpg --list-keys

mkdir -p $CWD/mirror

# create repo for indicated type and distribution
echo """#
# BOINC Repository
#

[boinc-$TYPE-$DISTRO]
name = BOINC $TYPE $DISTRO repository
baseurl = $BASEREPO/$TYPE/$DISTRO
arch = $ARCH
priority = 100
enabled = 1
gpgcheck = 1
gpgkey = $BASEREPO/$TYPE/$DISTRO/$RELEASEKEY
max_parallel_downloads = 2

""" > "$CWD/mirror/boinc-$TYPE-$DISTRO.repo"

# necessary for reposync to work correctly
mkdir -p /etc/yum/repos.d/
cp "$CWD/mirror/boinc-$TYPE-$DISTRO.repo" /etc/yum/repos.d/
dnf update -y -qq

# mirror the currently deployed repo (if any)
cd $CWD/mirror

reposync --nobest -a $ARCH --download-metadata --norepopath --repoid boinc-$TYPE-$DISTRO
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

cp $RPMSRC/*.rpm $CWD/mirror/
exit_on_fail "Failed to add new packages"

cd $CWD/mirror/
# keep only 4 last versions of each package
packets=$(find *.rpm | sort -t '_' -k 2 -V | uniq)
declare -A split_lists
packets_list=()
while IFS= read -r line; do
	packets_list+=("$line")
done <<< "$packets"
for item in "${packets_list[@]}"; do
	prefix=$(echo "$item" | cut -d '-' -f 1-2 ) # Extract the prefix (text before the second dash)
	split_lists["$prefix"]+="$item"$'\n'  # Append the item to the corresponding prefix's list
done

for prefix in "${!split_lists[@]}"; do
	echo "List for prefix: $prefix"
	echo "${split_lists[$prefix]}"
	count=$(echo "${split_lists[$prefix]}" | wc -l)
	number=$(expr $count - 1)
	echo "count=$number"
	i=0
	exceed=$(expr $number - 4)
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
				rm $value
				exit_on_fail "Failed to remove the package"
			else
				break
			fi
		done
	fi
done

if [[ ! "$IS_MIRROR" -eq "0" ]]; then
	createrepo_c .
	exit_on_fail "Failed to create repository"
else
	createrepo_c --update .
	exit_on_fail "Failed to update repository"
fi

# sign repository metadata
cd $CWD/mirror/repodata
gpg -s --default-key $HASH repomd.xml > repomd.xml.asc
exit_on_fail "Could not sign repository metadata"

cd $CWD/mirror/

# copy the key for the repo to the root of it
SRCKEYFILE="$SRC/$RELEASEKEY"
DSTKEYFILE="$CWD/mirror/$RELEASEKEY"
cp "${SRCKEYFILE}" "${DSTKEYFILE}"
exit_on_fail "Failed to publish the public key to the repo"

find .

# Archive the produced repo to the archive in the format expected by the upload script:
# repo-<stable/alpha>-<osversion>.tar.gz
tar -zcvf ${SRC}/repo-$TYPE-$DISTRO.tar.gz -C $CWD/mirror/ .
exit_on_fail "Could not package the repository for upload"

popd
