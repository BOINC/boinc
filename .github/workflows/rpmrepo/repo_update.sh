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
	printf "Usage: repo_update.sh <allow-create> <repo-url> <incoming-dir> [osversion] [release-type] [release-key] [key-hash]\n"
	exit 1
}

CWD=$(pwd)
TYPE=stable
DISTRO=fc38
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

reposync --nobest --download-metadata --norepopath --repoid boinc-$TYPE-$DISTRO
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

if [[ "$TYPE" == "stable" ]]; then
	# create alpha repo of the same distribution
	echo """#
# BOINC Repository
#

[boinc-alpha-$DISTRO]
name = BOINC alpha $DISTRO repository
baseurl = $BASEREPO/alpha/$DISTRO
priority = 100
enabled = 1
gpgcheck = 1
gpgkey = $BASEREPO/alpha/$DISTRO/$RELEASEKEY
max_parallel_downloads = 2

""" > "$CWD/mirror/boinc-alpha-$DISTRO.repo"

	# necessary for reposync to work correctly
	mkdir -p /etc/yum/repos.d/
	mv "$CWD/mirror/boinc-alpha-$DISTRO.repo" /etc/yum/repos.d/
	dnf update -y -qq

	# mirror the currently deployed alpha repo (if any)
	mkdir -p $CWD/alpha
	cd $CWD/alpha

	reposync --nobest --download-metadata --norepopath --repoid boinc-alpha-$DISTRO
	exit_on_fail "Could not mirror alpha ${REPO}"

	# keep only 1 last version of each package
	cd $CWD/alpha/
	packets=$(find *.rpm | sort -t '-' -k 2 -V -r | uniq)
    prefixes=$(echo "$packets"| cut -d '-' -f 1-2 | sort -n | uniq)
    suffixes=$(echo "$packets"| cut -d '.' -f 4 | sort -V | uniq)
    for prefix in $prefixes; do
        echo $prefix
        for suffix in $suffixes; do
            echo "  $suffix"
            matched_packets=$(echo "$packets" | grep "^${prefix}-.*\.${suffix}.rpm$")
            echo "$matched_packets" | tail -n 1 | while IFS= read -r packet; do
			echo "Copy: $packet"
			cp $value $CWD/mirror/
			exit_on_fail "Failed to copy the package $packet"
            done
        done
    done
else
	cp $RPMSRC/*.rpm $CWD/mirror/
	exit_on_fail "Failed to add new packages"
fi

cd $CWD/mirror/
# keep only 4 last versions of each package
packets=$(find *.rpm | sort -t '_' -k 2 -V | uniq)
prefixes=$(echo "$packets"| cut -d '-' -f 1-2 | sort -n | uniq)
suffixes=$(echo "$packets"| cut -d '.' -f 4 | sort -V | uniq)
for prefix in $prefixes; do
    echo $prefix
    for suffix in $suffixes; do
        echo "  $suffix"
        matched_packets=$(echo "$packets" | grep "^${prefix}-.*\.${suffix}.rpm$")
        count=$(echo "$matched_packets" | wc -l)
        echo "    count: $count"
        exceed=$(expr $count - 4)
        echo "    exceed: $exceed"
        if (( exceed > 0 )); then
            echo "$matched_packets" | head -n $exceed | while IFS= read -r packet; do
                echo "      Remove: $packet"
                rm $packet
			    exit_on_fail "Failed to remove the package"
            done
        fi
    done
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
