#!/bin/sh

# Script to build Macintosh Universal Binary library of wxMac-2.6.2 for
# use in building BOINC.
#
# by Charlie Fenton 12/19/05
#

# The following is optional.  It works around a bug in a script supplied 
# with wxMac-2.6.2.  Without these steps, the bug will force a full new 
# build every time the XCode project is run, even if not needed.
#
# Open [ path ]/wxMac-2.6.2/src/wxWindows.xcodeproj
#
# In the left hand column of the project window, use the disclosure
# triangles to view items in the hierarchy.
#
# In the left-hand column hierarchy, select (click on):
#     wxWindows/Targets/static/Run Script
# Type command-I to open the "Run Script" Info window.
# Select the General tab in the Info window
# Click on the + button under "Input Files" and replace the new entry with:
#     ${SRCROOT}/../include/wx/mac/setup.h
# Click on the + button under "Output Files" and replace the new entry with:
#     ${SYMROOT}/include/wx/setup.h
# Make sure the "Run script only when installing" checkbox is OFF.
#
# Quit XCode or close the project window.


# To build the wxMac-2.6.2 library for BOINC as a Universal Binary:
# In Terminal, CD to the wxMac-2.6.2 directory.
#    cd [path]/wxMac-2.6.2/
# then run this script:
#    source [ path_to_this_script ] [ -clean ]
#
# the -clean argument will force a full rebuild.
#

### THIS SCRIPT IS STILL UNDER DEVELOPMENT
### IT IS NOT YET RELIABLE
### IT DOESN'T DO DEVELOPMENT BUILD YET

if [ "$1" != "-clean" ]; then
##  if [ -f src/build/Deployment/libwx_mac.a ] && [ -f src/build/Development/libwx_macd.a ]; then
  if [ -f src/build/Deployment/libwx_mac.a ]; then
    echo "wxMac-2.6.2 already built"
    return 0
  fi
fi

if [ "$1" = "-clean" ]; then
  doclean="clean "
else
  doclean=""
fi

mv -n include/wx/mac/setup.h include/wx/mac/setup_obs.h
cp -np include/wx/mac/setup0.h include/wx/mac/setup.h

# Create wx include directory if necessary
if [ ! -d src/build/include/wx ]; then
    mkdir -p src/build/include/wx
fi

cp -n include/wx/mac/setup0.h src/build/include/wx/setup.h

xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Deployment $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk OTHER_CFLAGS="" OTHER_CPPFLAGS="" OTHER_CFLAGS_i386="-iquote ${SYMROOT}/include -iquote ../include -iquote mac/carbon/morefilex -iquote common -iquote jpeg -iquote png -iquote regex -iquote expat/lib -iquote tiff" OTHER_CPPFLAGS_i386="-iquote ${SYMROOT}/include -iquote ../include -iquote mac/carbon/morefilex -iquote common -iquote jpeg -iquote png -iquote regex -iquote expat/lib -iquote tiff"
if [  $? -ne 0 ]; then exit 1; fi

return 0


## **** THIS SHOULD WORK FOR DEPLOYMENT BUILD BUT DIDN'T: ??? ****

xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Deployment $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk OTHER_CFLAGS="" OTHER_CPLUSPLUSFLAGS="" PER_ARCH_CFLAGS_i386="-iquote ./build/include -iquote ../include -iquote mac/carbon/morefilex -iquote common -iquote jpeg -iquote png -iquote regex -iquote expat/lib -iquote tiff" PER_ARCH_CFLAGS_ppc="-I./build/include -I../include -Imac/carbon/morefilex -Icommon -Ijpeg -Ipng -Iregex -Iexpat/lib -Itiff" USE_SEPARATE_HEADERMAPS="NO" GCC_ENABLE_FIX_AND_CONTINUE="NO"

if [  $? -ne 0 ]; then exit 1; fi

## **** THIS ONE WORKS FOR INTEL DEVLOPMENT BUILD BUT NOT PPC: ****

xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Development $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk OTHER_CFLAGS="" OTHER_CPLUSPLUSFLAGS="" PER_ARCH_CFLAGS_i386="-iquote ./build/include -iquote ../include -iquote mac/carbon/morefilex -iquote common -iquote jpeg -iquote png -iquote regex -iquote expat/lib -iquote tiff" PER_ARCH_CFLAGS_ppc="-I./build/include -I../include -Imac/carbon/morefilex -Icommon -Ijpeg -Ipng -Iregex -Iexpat/lib -Itiff" USE_SEPARATE_HEADERMAPS="NO" GCC_ENABLE_FIX_AND_CONTINUE="NO"

if [  $? -ne 0 ]; then exit 1; fi

return 0
