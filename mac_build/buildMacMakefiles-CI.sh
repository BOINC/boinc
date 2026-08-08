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
#
#
# Script to build the different targets in the BOINC xcode project using a
# combined install directory for all dependencies
#
# Usage:
# ./mac_build/buildMacMakefiles-CI.sh [--cache_dir PATH] [--debug]
#
# --cache_dir is the path where the dependencies are installed by 3rdParty/buildMacDependencies.sh.
# --debug will build the debug Manager (needs debug wxWidgets library in cache_dir).

# check working directory because the script needs to be called like: ./mac_build/buildMacMakefiles-CI.sh
if [ ! -d "mac_build" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

# Delete any obsolete paths to old build products
rm -fR ./zip/build
rm -fR ./mac_build/build

cache_dir="$(pwd)/3rdParty/buildCache/mac"
doclean=""
style="Deployment"
config=""
beautifier="cat" # we need a fallback if xcpretty is not available
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        --debug|-dev)
        style="Development"
        config="-dev"
        ;;
    esac
    shift # past argument or value
done

if [ ! -d "$cache_dir" ] || [ ! -d "$cache_dir/lib" ] || [ ! -d "$cache_dir/include" ]; then
    echo "${cache_dir} is not a directory or does not contain dependencies"
fi

XCPRETTYPATH=`xcrun -find xcpretty 2>/dev/null`
if [ $? -eq 0 ]; then
    beautifier="xcpretty"
fi

cd ./mac_build || exit 1
retval=0

target="BOINC libs"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -lib -setting HEADER_SEARCH_PATHS "${cache_dir}/include \\\${HEADER_SEARCH_PATHS}" -setting USER_HEADER_SEARCH_PATHS "" -setting LIBRARY_SEARCH_PATHS "$libSearchPathDbg ${cache_dir}/lib \\\${LIBRARY_SEARCH_PATHS}" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc.a..."
lipo ./build/${style}/libboinc.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc.a...done"
echo "Verifying architecture (x86_64 arm64) of libjpeg.a..."
lipo ./build/${style}/libjpeg.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libjpeg.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libjpeg.a...done"
echo "Verifying architecture (x86_64 arm64) of libboinc_graphics2.a..."
lipo ./build/${style}/libboinc_graphics2.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_graphics2.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_graphics2.a...done"
echo "Verifying architecture (x86_64 arm64) of libboinc_api.a..."
lipo ./build/${style}/libboinc_api.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_api.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_api.a...done"
echo "Verifying architecture (x86_64 arm64) of libboinc_opencl.a..."
lipo ./build/${style}/libboinc_opencl.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_opencl.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_opencl.a...done"
echo "Verifying architecture (x86_64 arm64) of libboinc_zip.a..."
lipo ./build/${style}/libboinc_zip.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_zip.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_zip.a...done"
echo "Building ${target}...done"

target="Examples via Makefile"
echo "Building ${target}..."
cd ../samples/example_app
source MakeMacExample.sh --cache_dir ${cache_dir}; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of uc2..."
lipo ./x86_64/uc2 -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of uc2...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of uc2...done"
echo "Verifying architecture (arm64) of uc2..."
lipo ./arm64/uc2 -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of uc2...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of uc2...done"
echo "Verifying architecture (x86_64) of uc2_graphics..."
lipo ./x86_64/uc2_graphics -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of uc2_graphics...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of uc2_graphics...done"
echo "Verifying architecture (arm64) of uc2_graphics..."
lipo ./arm64/uc2_graphics -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of uc2_graphics...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of uc2_graphics...done"
echo "Verifying architecture (x86_64) of slide_show..."
lipo ./x86_64/slide_show -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of slide_show...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of slide_show...done"
echo "Verifying architecture (arm64) of slide_show..."
lipo ./arm64/slide_show -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of slide_show...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of slide_show...done"
cd ../../mac_build/
echo "Building ${target}...done"

target="Examples Makefile_mac2"
echo "Building ${target}..."
cd ../samples/example_app
export PREFIX=${cache_dir}
export CXX="${GPPPATH}"
SDKPATH=`xcodebuild -version -sdk macosx Path`
export SYSLIBROOT="-Wl,-syslibroot,${SDKPATH}"
export ISYSROOT="-isysroot ${SDKPATH}"
make -f Makefile_mac2 clean; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
make -f Makefile_mac2 all; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
export PREFIX=""
export CXX=""
export SYSLIBROOT=""
export ISYSROOT=""
echo "Verifying architecture (x86_64) of uc2_x86_64..."
lipo ./uc2_x86_64 -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of uc2_x86_64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of uc2_x86_64...done"
echo "Verifying architecture (arm64) of uc2_arm64..."
lipo ./uc2_arm64 -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of uc2_arm64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of uc2_arm64...done"
echo "Verifying architecture (x86_64) of uc2_graphics_x86_64..."
lipo ./uc2_graphics_x86_64 -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of uc2_graphics_x86_64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of uc2_graphics_x86_64...done"
echo "Verifying architecture (arm64) of uc2_graphics_arm64..."
lipo ./uc2_graphics_arm64 -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of uc2_graphics_arm64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of uc2_graphics_arm64...done"
echo "Verifying architecture (x86_64) of slide_show_x86_64..."
lipo ./slide_show_x86_64 -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of slide_show_x86_64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of slide_show_x86_64...done"
echo "Verifying architecture (arm64) of slide_show_arm64..."
lipo ./slide_show_arm64 -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of slide_show_arm64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of slide_show_arm64...done"
cd ../../mac_build/
echo "Building ${target}...done"

target="Wrapper"
echo "Building ${target}..."
cd ../samples/wrapper
source BuildMacWrapper.sh --cache_dir ${cache_dir}; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of wrapper..."
lipo ./wrapper -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of wrapper...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of wrapper...done"
cd ../../mac_build/
echo "Building ${target}...done"

target="VBoxWrapper"
echo "Building ${target}..."
cd ../samples/vboxwrapper
source BuildMacVboxWrapper.sh --cache_dir ${cache_dir}; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../../; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of vboxwrapper..."
lipo ./vboxwrapper -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of vboxwrapper...failed"
    echo "Building ${target}...failed"
    cd ../../; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of vboxwrapper...done"
cd ../../mac_build/
echo "Building ${target}...done"

target="openclapp"
echo "Building ${target}..."
cd ../samples/openclapp
export CXX="${GPPPATH}"
make -f Makefile_mac clean; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
make -f Makefile_mac all; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
export CXX=""
echo "Verifying architecture (x86_64) of openclapp_x86_64..."
lipo ./openclapp_x86_64 -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of openclapp_x86_64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of openclapp_x86_64...done"
echo "Verifying architecture (arm64) of openclapp_arm64..."
lipo ./openclapp_arm64 -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of openclapp_arm64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of openclapp_arm64...done"
cd ../../mac_build/
echo "Building ${target}...done"

target="docker_wrapper"
echo "Building ${target}..."
cd ../samples/docker_wrapper
export CXX="${GPPPATH}"
make -f Makefile_mac clean; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
make -f Makefile_mac all; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
export CXX=""
echo "Verifying architecture (x86_64) of docker_wrapper_x86_64..."
lipo ./docker_wrapper_x86_64 -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of docker_wrapper_x86_64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (x86_64) of docker_wrapper_x86_64...done"
echo "Verifying architecture (arm64) of docker_wrapper_arm64..."
lipo ./docker_wrapper_arm64 -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (arm64) of docker_wrapper_arm64...failed"
    echo "Building ${target}...failed"
    cd ../..; exit 1;
fi
echo "Verifying architecture (arm64) of docker_wrapper_arm64...done"
cd ../../mac_build/
echo "Building ${target}...done"

cd ..
