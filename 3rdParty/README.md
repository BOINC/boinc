# Purpose of 3rdParty

This directory is used to download and build dependencies for BOINC components.
Archives should be downloaded into a platform specific subdirectory (e.g linux, mac, ...)
to allow cross compilation of several platforms on a single host.

Build products of dependencies should be installed into a corresponding subdirectory of
3rdParty/buildCache to allow CI systems to cache this data between BOINC builds.

See the existing [buildLinuxDependencies.sh](buildLinuxDependencies.sh) and
[buildMacDependencies.sh](buildMacDependencies.sh) scripts for how this is done
on Linux and Mac currently.
