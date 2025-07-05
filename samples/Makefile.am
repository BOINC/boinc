## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

export WRAPPER_RELEASE_VERSION=$(WRAPPER_RELEASE)
export VBOXWRAPPER_RELEASE_VERSION=$(VBOXWRAPPER_RELEASE)
export WORKER_RELEASE_VERSION=$(WORKER_RELEASE)
export DOCKERWRAPPER_RELEASE_VERSION=$(DOCKERWRAPPER_RELEASE)
export MULTITHREAD_RELEASE_VERSION=$(MULTITHREAD_RELEASE)

if BUILD_WITH_MINGW
    export BUILD_WITH_MINGW="yes"
endif

if BUILD_WITH_GUI
    export BUILD_WITH_GUI="yes"
endif

if BUILD_APPS_WITH_VCPKG
    export BUILD_APPS_WITH_VCPKG="yes"
endif

## list the apps that should be build
## NOTE: nvcuda and wrappture need external libraries
SUBDIRS = condor example_app multi_thread sleeper sporadic worker wrapper

if BUILD_WITH_VBOX
    SUBDIRS += vboxmonitor vboxwrapper
endif

if BUILD_APPS_WITH_VCPKG
    SUBDIRS += wrappture
endif

## build openclapp if you have opencl on your device
if BUILD_WITH_OPENCL
    SUBDIRS += openclapp
endif

if ! ANDROID
    SUBDIRS += docker_wrapper
endif

maintainer-clean:
