## -*- mode: makefile; tab-width: 4 -*-
EXTRA_DIST = make-sea.sh

vers = \
  @BOINC_MAJOR_VERSION@.@BOINC_MINOR_VERSION@.@BOINC_RELEASE@_@boinc_platform@
sea = boinc_$(vers).sh
tarfiles = BOINC/boinc BOINC/boincmgr BOINC/boinccmd BOINC/binstall.sh \
	BOINC/boincmgr.16x16.png BOINC/boincmgr.32x32.png BOINC/boincmgr.48x48.png \
	BOINC/ca-bundle.crt

if HAVE_CUDA_LIB
tarfiles += BOINC/libcudart.so
BOINC/libcudart.so: $(top_builddir)/coprocs/CUDA/posix/@boinc_platform@/libcudart.so BOINC
	cp $(top_builddir)/coprocs/CUDA/posix/@boinc_platform@/libcudart.so BOINC/libcudart.so
endif

all: $(top_builddir)/stage $(sea)

# WARNING: WHAT FOLLOWS IS 'FRAGILE'.  DO NOT REPLACE
# TABS WITH SPACES!

$(top_builddir)/stage:
	cd $(top_builddir) ; make stage

sea.tar: $(tarfiles)
	./make-tar.sh ${top_builddir}

$(top_builddir)/stage/$(bindir)/boinc: $(top_builddir)/stage

$(top_builddir)/stage/$(bindir)/boincmgr: $(top_builddir)/stage

$(top_builddir)/stage/$(bindir)/boinccmd: $(top_builddir)/stage

$(sea): sea.tar make-sea.sh Makefile
	./make-sea.sh sea.tar $(sea) BOINC/binstall.sh


BOINC/boinc: $(top_builddir)/stage/$(bindir)/boinc BOINC
	cp $(top_builddir)/stage/$(bindir)/boinc BOINC/boinc

BOINC/boincmgr: $(top_builddir)/stage/$(bindir)/boincmgr BOINC
	cp $(top_builddir)/stage/$(bindir)/boincmgr BOINC/boincmgr

BOINC/boinccmd: $(top_builddir)/stage/$(bindir)/boinccmd BOINC
	cp $(top_builddir)/stage/$(bindir)/boinccmd BOINC/boinccmd

BOINC/boincmgr.16x16.png: boincmgr.16x16.png
	cp boincmgr.16x16.png BOINC/boincmgr.16x16.png
BOINC/boincmgr.32x32.png: boincmgr.32x32.png
	cp boincmgr.32x32.png BOINC/boincmgr.32x32.png
BOINC/boincmgr.48x48.png: boincmgr.48x48.png
	cp boincmgr.48x48.png BOINC/boincmgr.48x48.png
BOINC/ca-bundle.crt: $(top_builddir)/curl/ca-bundle.crt
	cp $(top_builddir)/curl/ca-bundle.crt BOINC/ca-bundle.crt

#BOINC_debug/boinc.unmodified: $(top_builddir)/stage/$(bindir)/boinc.unmodified BOINC_debug
#	cp $(top_builddir)/stage/$(bindir)/boinc.unmodified BOINC_debug/boinc.unmodified
#
#BOINC_debug/boincmgr.unmodified: $(top_builddir)/stage/$(bindir)/boincmgr.unmodified BOINC_debug
#	cp $(top_builddir)/stage/$(bindir)/boincmgr.unmodified BOINC_debug/boincmgr.unmodified
#
#BOINC_debug/boinccmd.unmodified: $(top_builddir)/lib/boinccmd.unmodified BOINC_debug
#	cp $(top_builddir)/lib/boinccmd.unmodified BOINC_debug/boinccmd.unmodified
#
#BOINC_debug/libcudart.so: $(top_builddir)/coprocs/CUDA/posix/@boinc_platform@/libcudart.so BOINC_debug
#	cp $(top_builddir)/coprocs/CUDA/posix/@boinc_platform@/libcudart.so BOINC_debug/libcudart.so
#
#BOINC_debug/boincmgr.16x16.png: boincmgr.16x16.png
#	cp boincmgr.16x16.png BOINC_debug/boincmgr.16x16.png
#BOINC_debug/boincmgr.32x32.png: boincmgr.32x32.png
#	cp boincmgr.32x32.png BOINC_debug/boincmgr.32x32.png
#BOINC_debug/boincmgr.48x48.png: boincmgr.48x48.png
#	cp boincmgr.48x48.png BOINC_debug/boincmgr.48x48.png
#BOINC_debug/ca-bundle.crt: $(top_builddir)/curl/ca-bundle.crt
#	cp $(top_builddir)/curl/ca-bundle.crt BOINC_debug/ca-bundle.crt
 
BOINC/binstall.sh: BOINC Makefile
	echo 'cd BOINC &&\
	echo "cd \"$$PWD\" && exec ./boinc \$$@" > run_client &&\
	echo "cd \"$$PWD\" && exec ./boincmgr \$$@" > run_manager &&\
	chmod +x run_client &&\
	chmod +x run_manager &&\
	echo use $$PWD/run_manager to start BOINC' > BOINC/binstall.sh

#BOINC_debug/binstall.sh: BOINC_debug Makefile
#	echo 'cd BOINC_debug &&\
#	echo "cd \"$$PWD\" && exec ./boinc.unmodified \$$@" > run_client &&\
#	echo "cd \"$$PWD\" && exec ./boincmgr.unmodified \$$@" > run_manager &&\
#	chmod +x run_client &&\
#	chmod +x run_manager &&\
#	echo use $$PWD/run_manager to start BOINC' > BOINC_debug/binstall.sh

BOINC:
	mkdir -p BOINC

#BOINC_debug:
#	mkdir -p BOINC_debug

clean:
	rm -rf BOINC sea.tar $(sea)
	rm -rf BOINC_debug sea_debug.tar $(sea_debug)

