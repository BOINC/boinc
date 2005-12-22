# To build the wxMac-2.6.2 library for BOINC as a Universal Binary:
#
# Before running this script:
#
# Open wxWindows.xcode to upgrade it for XCode 2.2, and name the new file 
#    wxWindowsB.xcodeproj.
# Do NOT use the wxWindows.xcodeproj supplied with the package.
#
# In Terminal, CD to the wxMac-2.6.2 directory.
#    cd [path]/wxMac-2.6.2/
# then run this script:
#    source [ path_to_this_script ] [ -clean ]
#
# the -clean argument will force a full rebuild.
#

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


if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_mac_ppc.a ]; then
    echo "libwx_mac_ppc.a already built"
else

rm -f src/build/Deployment/libwx_mac.a

xcodebuild -project src/wxWindowsB.xcodeproj -target wxStaticRelease  -configuration Deployment $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk ARCHS="ppc" EXECUTABLE_NAME="libwx_mac_ppc.a"

if [  $? -ne 0 ]; then exit 1; fi
fi


if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_mac_ppc.a ]; then
    echo "libwx_mac_i386.a already built"
else

rm -f src/build/Deployment/libwx_mac.a

xcodebuild -project src/wxWindowsB.xcodeproj -target wxStaticRelease  -configuration Deployment $doclean build GCC_VERSION_i386=4.0 MACOSX_DEPLOYMENT_TARGET_i386=10.4 SDKROOT_i386=/Developer/SDKs/MacOSX10.4u.sdk ARCHS="i386" EXECUTABLE_NAME="libwx_mac_i386.a"

if [  $? -ne 0 ]; then exit 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_mac.a ]; then
    echo "libwx_mac.a already built"
else

lipo -create src/build/Deployment/libwx_mac_ppc.a src/build/Deployment/libwx_mac_i386.a -output src/build/Deployment/libwx_mac.a

if [  $? -ne 0 ]; then exit 1; fi
fi


if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_macd_ppc.a ]; then
    echo "libwx_macd_ppc.a already built"
else

rm -f src/build/Deployment/libwx_macd.a

xcodebuild -project src/wxWindowsB.xcodeproj -target wxStaticDebug  -configuration Deployment $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk ARCHS="ppc" EXECUTABLE_NAME="libwx_macd_ppc.a"

if [  $? -ne 0 ]; then exit 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_macd_i386.a ]; then
    echo "libwx_macd_i386.a already built"
else

rm -f src/build/Deployment/libwx_macd.a

xcodebuild -project src/wxWindowsB.xcodeproj -target wxStaticDebug  -configuration Deployment $doclean build GCC_VERSION_i386=4.0 MACOSX_DEPLOYMENT_TARGET_i386=10.4 SDKROOT_i386=/Developer/SDKs/MacOSX10.4u.sdk ARCHS="i386" EXECUTABLE_NAME="libwx_macd_i386.a"

if [  $? -ne 0 ]; then exit 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_macd_ppc.a ]; then
    echo "libwx_macd_ppc.a already built"
else

lipo -create src/build/Deployment/libwx_macd_ppc.a src/build/Deployment/libwx_macd_i386.a -output src/build/Deployment/libwx_macd.a

if [  $? -ne 0 ]; then exit 1; fi
fi

return 0

