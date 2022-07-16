## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

if BUILD_WITH_MINGW
    export BUILD_WITH_MINGW="yes"
endif

if BUILD_WITH_GUI
    export BUILD_WITH_GUI="yes"
endif

if BUILD_WITH_VCPKG
    export BUILD_WITH_VCPKG="yes"
endif

## list the apps that should be build
## NOTE: nvcuda and wrappture need external libraries
SUBDIRS = condor example_app multi_thread sleeper worker wrapper

if BUILD_WITH_VBOX
    SUBDIRS += vboxmonitor vboxwrapper
endif

if BUILD_WITH_VCPKG
    SUBDIRS += wrappture
endif

## build openclapp if you have opencl on your device
if BUILD_WITH_OPENCL
    SUBDIRS += openclapp
endif

maintainer-clean:
