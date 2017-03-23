#!/bin/bash

## This file contains the names and download URLs for the 7 third-party
## libraries needed for building BOINC for Macintosh computers.
##
## This file is usually source'ed by scripts that build the dependencies.
##
## Update this file and the Xcode project build settings when upgrading
## to a newer version of one of these libraries. Note that the scripts
## buildcurl,sh and buildWxMac.sh contain code to patch source files
## for their respective libraries, so they may need to be modified
## for new versions of those source files.
##
## To ensure that rebuilds of older versions of BOINC always use the
## same versions of the third party libraries as the original builds,
## always hard-code the path to the correct libraries in the Xcode build
## settings; please do NOT include or reference this file in the Xcode
## build settings.
##

opensslDirName="openssl-1.1.0"
opensslFileName="openssl-1.1.0.tar.gz"
opensslURL="https://www.openssl.org/source/openssl-1.1.0.tar.gz"

caresDirName="c-ares-1.11.0"
caresFileName="c-ares-1.11.0.tar.gz"
caresURL= "http://c-ares.haxx.se/download/c-ares-1.11.0.tar.gz"

curlDirName="curl-7.50.2"
curlFileName="curl-7.50.2.tar.gz"
curlURL="http://curl.haxx.se/download/curl-7.50.2.tar.gz"

wxWidgetsDirName="wxWidgets-3.0.0"
wxWidgetsFileName="wxWidgets-3.0.0.tar.bz2"
wxWidgetsURL="http://sourceforge.net/projects/wxwindows/files/3.0.0/wxWidgets-3.0.0.tar.bz2"

sqliteDirName="sqlite-autoconf-3110000"
sqliteFileName="sqlite-autoconf-3110000.tar.gz"
sqliteURL="http://www.sqlite.org/2016/sqlite-autoconf-3110000.tar.gz"

freetypeDirName="freetype-2.6.2"
freetypeFileName="freetype-2.6.2.tar.bz2"
freetypeURL="http://sourceforge.net/projects/freetype/files/freetype2/2.6.2/freetype-2.6.2.tar.bz2"

ftglDirName="ftgl-2.1.3~rc5"
ftglFileName="ftgl-2.1.3-rc5.tar.gz"
ftglURL="http://sourceforge.net/projects/ftgl/files/FTGL%20Source/2.1.3%7Erc5/ftgl-2.1.3-rc5.tar.gz"

return 0
