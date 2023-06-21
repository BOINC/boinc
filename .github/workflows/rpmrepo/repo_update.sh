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

RPMSRC="$SRC/rpmbuild/RPMS/$ARCH"

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
