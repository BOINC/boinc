#!/usr/bin/env bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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

# Run package installation tests with systemd and journald as on a real host.
set -euo pipefail

image=${1:?Usage: start_linux_package_test_container.sh IMAGE}
: "${GITHUB_WORKSPACE:?}"
: "${RUNNER_TEMP:?}"

docker build --build-arg "IMAGE=$image" --tag boinc-package-test - <<'DOCKERFILE'
ARG IMAGE=debian:bookworm
FROM ${IMAGE}
RUN if command -v apt-get >/dev/null; then \
        apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y systemd systemd-sysv dbus && rm -rf /var/lib/apt/lists/*; \
    elif command -v dnf >/dev/null; then \
        dnf install -y systemd dbus && dnf clean all; \
    else \
        zypper --non-interactive install systemd dbus-1 && zypper clean --all; \
    fi; \
    if [ ! -e /usr/lib/systemd/systemd-sysv-install ]; then \
        ln -s /bin/true /usr/lib/systemd/systemd-sysv-install; \
    fi; \
    rm -f /usr/sbin/policy-rc.d
STOPSIGNAL SIGRTMIN+3
DOCKERFILE

# Keep cgroups private to the test container; do not mount the host's /run.
docker run --detach --name boinc-package-test --privileged --cgroupns=private \
    --tmpfs /run --tmpfs /run/lock --tmpfs /tmp \
    --volume "$GITHUB_WORKSPACE:$GITHUB_WORKSPACE" \
    --volume "$RUNNER_TEMP:$RUNNER_TEMP" \
    --workdir "$GITHUB_WORKSPACE" boinc-package-test /sbin/init

for ((attempt = 0; attempt < 30; attempt++)); do
    state=$(docker exec boinc-package-test systemctl is-system-running || true)
    if [[ "$state" == running || "$state" == degraded ]]; then
        if docker exec boinc-package-test systemctl is-active --quiet systemd-journald.service; then
            exit 0
        fi
    fi
    sleep 1
done

docker logs boinc-package-test
docker exec boinc-package-test systemctl --failed || true
echo "systemd/journald did not become ready" >&2
exit 1
