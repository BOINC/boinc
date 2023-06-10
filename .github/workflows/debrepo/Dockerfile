# example build usage:
# LINUX: export BUILDKIT_PROGRESS=plain 
# WINDOWS: set BUILDKIT_PROGRESS=plain 
# docker build -t jammy-boinc --build-arg PACKAGE=boinc-linux-client --build-arg VERSION=1.0.0-1 -f ./Dockerfile .

ARG DISTRO=ubuntu
ARG RELEASE=jammy

FROM $DISTRO:$RELEASE

# All args are cleared after a FROM instruction
ARG RELEASE=jammy

ARG REPOBASE=https://boinc.berkeley.edu/dl/linux
ARG REPOTYPE=stable
ARG PACKAGE=boinc-linux-client
ARG VERSION=1.0.0-1
ARG REPOKEY=boinc-202305.gpg

USER root

WORKDIR /root

RUN bash -c 'apt update && apt-get install -y wget gnupg ca-certificates'

RUN bash -c 'wget $REPOBASE/$REPOTYPE/$RELEASE/$REPOKEY'

RUN bash -c 'apt-key add $REPOKEY'

RUN bash -c 'echo "deb $REPOBASE/$REPOTYPE/$RELEASE $RELEASE main" >> /etc/apt/sources.list'

RUN bash -c 'cat /etc/apt/sources.list'

RUN bash -c 'apt update'

RUN bash -c 'apt list -a $PACKAGE'

RUN bash -c 'apt-get install -y $PACKAGE=$VERSION'
