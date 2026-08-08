#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2017 University of California
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

# Script to build a wxWidgets GTK version for BOINC

# Usage:
#  cd [path]/wxWidgets-3.0.5/
#  source path_to_this_script [--clean] [--debug] [--prefix PATH]
#
# the --clean argument will force a full rebuild.
# the --debug argument will build the debug version of the library
# if --prefix is given as absolute path the library is installed into there

# Patch wxwidgets to work with GCC6
# from https://github.com/wxWidgets/wxWidgets/commit/73e9e18ea09ffffcaac50237def0d9728a213c02
if [ ! -f  src/stc/scintilla/src/Editor.cxx.orig ]; then
    cat >> /tmp/Editor.cxx.patch << ENDOFFILE
--- Editor.cxx  2014-10-06 23:33:44.000000000 +0200
+++ Editor_patched.cxx  2017-03-20 10:24:14.776685161 +0100
@@ -11,6 +11,7 @@
 #include <ctype.h>
 #include <assert.h>

+#include <cmath>
 #include <string>
 #include <vector>
 #include <map>
@@ -5841,9 +5842,9 @@
 }

 static bool Close(Point pt1, Point pt2) {
-       if (abs(pt1.x - pt2.x) > 3)
+       if (std::abs(pt1.x - pt2.x) > 3)
                return false;
-       if (abs(pt1.y - pt2.y) > 3)
+       if (std::abs(pt1.y - pt2.y) > 3)
                return false;
        return true;
 }
ENDOFFILE
    patch -blfu src/stc/scintilla/src/Editor.cxx /tmp/Editor.cxx.patch
    rm -f /tmp/Editor.cxx.patch
else
    echo "src/stc/scintilla/src/Editor.cxx already patched"
fi

doclean=""
debug_flag="--disable-debug_flag"
webview_flag="--enable-webview"
lprefix=""
cmdline_prefix=""
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -debug|--debug)
        debug_flag="--enable-debug"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        cmdline_prefix="--prefix=${lprefix}"
        shift
        ;;
        --disable-webview)
        webview_flag="--disable-webview"
        ;;
    esac
    shift # past argument or value
done

if [ -d buildgtk ] && [ "${doclean}" = "yes" ]; then
    rm -rf buildgtk
fi
mkdir -p buildgtk
cd buildgtk || return 1

../configure "${cmdline_prefix}" --with-gtk --disable-shared ${webview_flag} --disable-gtktest --disable-sdltest ${debug_flag}
if [ $? -ne 0 ]; then cd ..; return 1; fi
make 1>/dev/null # the wxWidgets build is very noisy so tune it down to warnings and errors only
if [ $? -ne 0 ]; then cd ..; return 1; fi
if [ "x${lprefix}" != "x" ]; then
    make install
    if [ $? -ne 0 ]; then cd ..; return 1; fi
fi

cd ..
return 0
