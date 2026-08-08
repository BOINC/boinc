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

# make_certificate_updater.sh
##
## Script to build installer for BOINC certificate update for OS 10.3.9 - OS 10.5.8
##
## Created 4/28/18 by Charlie Fenton
##
## NOTE: this script must be run on an OS 10.5 or OS 10.6 system which has
## Xcode 3 and PackageMaker 3 installed.
##
## Usage:
## cd to the root directory of the boinc tree, then invoke this script. For example:
##     cd [path]/boinc
##     source [path_to_this_script]
##

rm -fR /tmp/pkg_root
mkdir -p "/tmp/pkg_root/Library/Application Support/BOINC Data"
cp -fp ./curl/ca-bundle.crt "/tmp/pkg_root/Library/Application Support/BOINC Data/"

rm -fR /tmp/pkg_scripts
mkdir -p "/tmp/pkg_scripts"
cat >> /tmp/pkg_scripts/postinstall << ENDOFFILE
#!/bin/bash

chmod 0664 "/Library/Application Support/BOINC Data/ca-bundle.crt"
chown boinc_master:boinc_master "/Library/Application Support/BOINC Data/ca-bundle.crt"
ENDOFFILE

cp -f "/tmp/pkg_scripts/postinstall" "/tmp/pkg_scripts/postupgrade"

rm -fR "../BOINC_Installer/upate_boinc_certificate.pkg"
mkdir -p ../BOINC_Installer

/Developer/usr/bin/packagemaker -r "/tmp/pkg_root/" -s "/tmp/pkg_scripts/" -i ".edu.berkeley.boinc-cert-updater.pkg" -o "../BOINC_Installer/update_boinc_certificate.pkg" -g 10.3 -b -w

rm -fR /tmp/pkg_root
rm -fR /tmp/pkg_scripts
