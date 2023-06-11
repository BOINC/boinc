#!/bin/bash -xe

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
	printf "Usage: repo_update.sh <remove-pkg> <repo-url> <incoming-dir> [osversion(jammy,focal,buster,bullseye)] [release-type(stable,alpha)] [release-key]\n"
	exit 1
}

CWD=$(pwd)
TYPE=stable
DISTRO=jammy
RELEASEKEY=boinc-202305.gpg

# commandline params
DEL_PACKAGE=$1

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

# mirror the currently deployed repo (fail if not existing)
aptly -config=$CONF_FILE -keyring=$KEYRING mirror create boinc-$TYPE-mirror $REPO $DISTRO
exit_on_fail "Mirror could not be created"

# updates the the packages from remote
aptly -config=$CONF_FILE -keyring=$KEYRING mirror update boinc-$TYPE-mirror
exit_on_fail "Failed to update the local mirror"

	# imports the downloaded packages to the local mirror
aptly -config=$CONF_FILE repo import boinc-$TYPE-mirror boinc-$TYPE "Name"
exit_on_fail "Failed to import the remote mirror into local"

# info about the repo
aptly -config=$CONF_FILE repo show old-boinc-$TYPE-snap


# Removes the indicated package
aptly -config=$CONF_FILE repo remove boinc-$TYPE "$DEL_PACKAGE"
exit_on_fail "Failed to remove the package"


# create new snapshot of the repo for deployment (with mirror)
aptly -config=$CONF_FILE snapshot create boinc-$TYPE-snap from repo boinc-$TYPE
exit_on_fail "Failed to create new snapshot of the local repo"

# shows the contents of the new snapshot
aptly -config=$CONF_FILE snapshot show boinc-$TYPE-snap

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
