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
ARG REPOKEY=boinc.gpg

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
