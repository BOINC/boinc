## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

## list the apps that should be build
## NOTE: nvcuda and wrappture need external libraries
SUBDIRS = condor example_app multi_thread sleeper vboxmonitor vboxwrapper worker wrapper

## build openclapp if you have opencl on your device
if BUILD_OPENCL
SUBDIRS += openclapp
endif

maintainer-clean:
