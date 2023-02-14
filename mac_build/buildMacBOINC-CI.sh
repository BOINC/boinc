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
#
#
# Script to build the different targets in the BOINC xcode project using a
# combined install directory for all dependencies
#
# Usage:
# ./mac_build/buildMacBOINC-CI.sh [--cache_dir PATH] [--debug] [--clean] [--no_shared_headers]
#
# --cache_dir is the path where the dependencies are installed by 3rdParty/buildMacDependencies.sh.
# --debug will build the debug Manager (needs debug wxWidgets library in cache_dir).
# --clean will force a full rebuild.
# --no_shared_headers will build targets individually instead of in one call of BuildMacBOINC.sh (NOT recommended)

# check working directory because the script needs to be called like: ./mac_build/buildMacBOINC-CI.sh
if [ ! -d "mac_build" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

# Delete any obsolete paths to old build products
rm -fR ./zip/build
rm -fR ./mac_build/build

cache_dir="$(pwd)/3rdParty/buildCache/mac"
style="Deployment"
config=""
doclean=""
beautifier="cat" # we need a fallback if xcpretty is not available
share_paths="yes"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        --debug|-dev)
        style="Development"
        config="-dev"
        ;;
        --no_shared_headers)
        share_paths="no"
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

if [ ${share_paths} = "yes" ]; then
    ## all targets share the same header and library search paths
    libSearchPathDbg=""
    if [ "${style}" == "Development" ]; then
        libSearchPathDbg="./build/Development  ${cache_dir}/lib/debug"
    fi
    source BuildMacBOINC.sh ${config} -all -setting HEADER_SEARCH_PATHS "../clientgui ${cache_dir}/include ../samples/jpeglib ${cache_dir}/include/freetype2" -setting USER_HEADER_SEARCH_PATHS "" -setting LIBRARY_SEARCH_PATHS "$libSearchPathDbg ${cache_dir}/lib ../lib" | tee xcodebuild_all.log | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then
        cd ..; exit 1; fi
    return 0
fi

## This is code that builds each target individually in case the above shared header paths version is giving problems
## Note: currently this does not build the boinc_zip library
if [ "${doclean}" = "yes" ]; then
    ## clean all targets
    xcodebuild -project boinc.xcodeproj -target Build_All  -configuration ${style} clean | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then cd ..; exit 1; fi

    ## clean boinc_zip which is not included in Build_All
    xcodebuild -project ../zip/boinc_zip.xcodeproj -target boinc_zip -configuration ${style} clean | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then cd ..; exit 1; fi
fi

## Target mgr_boinc also builds dependent targets SetVersion and BOINC_Client
libSearchPathDbg=""
if [ "${style}" == "Development" ]; then
    libSearchPathDbg="${cache_dir}/lib/debug"
fi
target="mgr_boinc"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} -setting HEADER_SEARCH_PATHS "../clientgui ${cache_dir}/include" -setting LIBRARY_SEARCH_PATHS "${libSearchPathDbg} ${cache_dir}/lib" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
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
echo "Verifying architecture (x86_64 arm64) of BOINCManager..."
lipo ./build/${style}/BOINCManager.app/Contents/MacOS/BOINCManager -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of BOINCManager...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of BOINCManager...done"
echo "Verifying architecture (x86_64 arm64) of SetVersion..."
lipo ./build/${style}/SetVersion -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of SetVersion...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of SetVersion...done"
echo "Verifying architecture (x86_64 arm64) of boinc..."
lipo ./build/${style}/boinc -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of boinc...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boinc...done"
echo "Verifying architecture (x86_64) of detect_rosetta_cpu..."
lipo ./build/${style}/detect_rosetta_cpu -verify_arch x86_64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64) of detect_rosetta_cpu...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64) of detect_rosetta_cpu...done"
echo "Verifying architecture (arm64) of detect_rosetta_cpu..."
lipo ./build/${style}/detect_rosetta_cpu -verify_arch arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -eq 0 ]; then
    echo "Verifying architecture (arm64) of detect_rosetta_cpu...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (arm64) of detect_rosetta_cpu...done"
echo "Building ${target}...done"

## Target gfx2libboinc also build dependent target jpeg
target="gfx2libboinc"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} -setting HEADER_SEARCH_PATHS "../samples/jpeglib" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
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
echo "Building ${target}...done"

target="libboinc"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
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
echo "Building ${target}...done"

target="api_libboinc"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_api.a..."
lipo ./build/${style}/libboinc_api.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_api.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_api.a...done"
echo "Building ${target}...done"

target="PostInstall"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boinc_finish_install..."
lipo ./build/${style}/boinc_finish_install -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of boinc_finish_install...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boinc_finish_install...done"
echo "Verify architecture (x86_64 arm64) of SetVersion..."
lipo ./build/${style}/SetVersion -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verify architecture (x86_64 arm64) of SetVersion...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verify architecture (x86_64 arm64) of SetVersion...done"
echo "Verify architecture (x86_64 arm64) of PostInstall..."
lipo ./build/${style}/PostInstall.app/Contents/MacOS/PostInstall -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verify architecture (x86_64 arm64) of PostInstall...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verify architecture (x86_64 arm64) of PostInstall...done"
echo "Building ${target}...done"

target="switcher"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of switcher..."
lipo ./build/${style}/switcher -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of switcher...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of switcher...done"
echo "Building ${target}...done"

target="gfx_switcher"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of gfx_switcher..."
lipo ./build/${style}/gfx_switcher -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of gfx_switcher...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of gfx_switcher...done"
echo "Building ${target}...done"

target="Install_BOINC"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of Install_BOINC..."
lipo ./build/${style}/BOINC\ Installer.app/Contents/MacOS/BOINC\ Installer -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of Install_BOINC...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of Install_BOINC...done"
echo "Building ${target}...done"

libSearchPath=""
if [ "${style}" == "Development" ]; then
   libSearchPath="./build/Development"
fi
target="ss_app"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} -setting HEADER_SEARCH_PATHS "../api/ ../samples/jpeglib/ ${cache_dir}/include ${cache_dir}/include/freetype2"  -setting LIBRARY_SEARCH_PATHS "${libSearchPath} ${cache_dir}/lib ../lib" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libjpeg.a..."
lipo ./build/${style}/libjpeg.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libjpeg.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libjpeg.a...done"
echo "Verifying architecture (x86_64 arm64) of boincscr..."
lipo ./build/${style}/boincscr -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of boincscr...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boincscr...done"
echo "Building ${target}...done"

target="ScreenSaver"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} -setting GCC_ENABLE_OBJC_GC "unsupported" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of gfx_cleanup..."
lipo ./build/${style}/gfx_cleanup -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of gfx_cleanup...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of gfx_cleanup...done"
echo "Verifying architecture (x86_64 arm64) of BOINCSaver..."
lipo ./build/${style}/BOINCSaver.saver/Contents/MacOS/BOINCSaver -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of BOINCSaver...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of BOINCSaver...done"
echo "Building ${target}...done"

target="boinc_opencl"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_opencl.a..."
lipo ./build/${style}/libboinc_opencl.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_opencl.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_opencl.a...done"
echo "Building ${target}...done"

target="setprojectgrp"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of setprojectgrp..."
lipo ./build/${style}/setprojectgrp -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of setprojectgrp...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of setprojectgrp...done"
echo "Building ${target}...done"

target="cmd_boinc"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boinccmd..."
lipo ./build/${style}/boinccmd -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of boinccmd...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boinccmd...done"
echo "Building ${target}...done"

target="Uninstaller"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of Uninstaller..."
lipo ./build/${style}/Uninstall\ BOINC.app/Contents/MacOS/Uninstall\ BOINC -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of Uninstaller...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of Uninstaller...done"
echo "Building ${target}...done"

target="SetUpSecurity"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of SetUpSecurity..."
lipo ./build/${style}/SetUpSecurity -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of SetUpSecurity...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of SetUpSecurity...done"
echo "Building ${target}...done"

target="AddRemoveUser"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -target ${target} | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of AddRemoveUser..."
lipo ./build/${style}/AddRemoveUser -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of AddRemoveUser...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of AddRemoveUser...done"
echo "Building ${target}...done"

target="zip apps"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -zipapps | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_zip.a..."
lipo ./build/${style}/libboinc_zip.a -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of libboinc_zip.a...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of libboinc_zip.a...done"
echo "Verifying architecture (x86_64 arm64) of boinc_zip_test..."
lipo ../zip/build/${style}/boinc_zip_test -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of boinc_zip_test...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of boinc_zip_test...done"
echo "Verifying architecture (x86_64 arm64) of testzlibconflict..."
lipo ../zip/build/${style}/testzlibconflict -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of testzlibconflict...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of testzlibconflict...done"
echo "Building ${target}...done"

target="UpperCase2"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -uc2 -setting HEADER_SEARCH_PATHS "../../ ../../api/ ../../lib/ ../../zip/ ../../clientgui/mac/ ../jpeglib/ ../samples/jpeglib/ ${cache_dir}/include ${cache_dir}/include/freetype2"  -setting LIBRARY_SEARCH_PATHS "../../mac_build/build/Deployment ${cache_dir}/lib" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of UC2_graphics-apple-darwin..."
lipo ../samples/mac_build/build/${style}/UC2_graphics-apple-darwin -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of UC2_graphics-apple-darwin...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of UC2_graphics-apple-darwin...done"
echo "Verifying architecture (x86_64 arm64) of UC2-apple-darwin..."
lipo ../samples/mac_build/build/${style}/UC2-apple-darwin -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of UC2-apple-darwin...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of UC2-apple-darwin...done"
echo "Verifying architecture (x86_64 arm64) of slide_show-apple-darwin..."
lipo ../samples/mac_build/build/${style}/slide_show-apple-darwin -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of slide_show-apple-darwin...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of slide_show-apple-darwin...done"
echo "Building ${target}...done"

target="VBoxWrapper"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} -noclean -vboxwrapper -setting HEADER_SEARCH_PATHS "../../ ../../api/ ../../lib/ ../../clientgui/mac/ ../samples/jpeglib ${cache_dir}/include"  -setting LIBRARY_SEARCH_PATHS "../../mac_build/build/Deployment ${cache_dir}/lib" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of vboxwrapper..."
lipo ../samples/vboxwrapper/build/${style}/vboxwrapper -verify_arch x86_64 arm64 | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Verifying architecture (x86_64 arm64) of vboxwrapper...failed"
    echo "Building ${target}...failed"
    cd ..; exit 1;
fi
echo "Verifying architecture (x86_64 arm64) of vboxwrapper...done"
echo "Building ${target}...done"

cd ..
