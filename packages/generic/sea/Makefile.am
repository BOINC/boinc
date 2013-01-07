## -*- mode: makefile; tab-width: 4 -*-
EXTRA_DIST = make-sea.sh

vers = \
  @BOINC_MAJOR_VERSION@.@BOINC_MINOR_VERSION@.@BOINC_RELEASE@_@boinc_platform@
sea = boinc_$(vers).sh
tarfiles = BOINC/boinc BOINC/boincmgr BOINC/boincscr BOINC/boinccmd BOINC/binstall.sh \
	BOINC/boincmgr.16x16.png BOINC/boincmgr.32x32.png BOINC/boincmgr.48x48.png \
	BOINC/ca-bundle.crt

all: $(top_builddir)/stage $(sea)

# WARNING: WHAT FOLLOWS IS 'FRAGILE'.  DO NOT REPLACE
# TABS WITH SPACES!

$(top_builddir)/stage:
	cd $(top_builddir) ; make stage

sea.tar: $(tarfiles)
	./make-tar.sh ${top_builddir}

$(top_builddir)/stage/$(bindir)/boinc: $(top_builddir)/stage

$(top_builddir)/stage/$(bindir)/boincmgr: $(top_builddir)/stage

$(top_builddir)/stage/$(bindir)/boincscr: $(top_builddir)/stage

$(top_builddir)/stage/$(bindir)/boinccmd: $(top_builddir)/stage

$(sea): sea.tar make-sea.sh Makefile
	./make-sea.sh sea.tar $(sea) BOINC/binstall.sh


BOINC/boinc: $(top_builddir)/stage/$(bindir)/boinc BOINC
	cp $(top_builddir)/stage/$(bindir)/boinc BOINC/boinc

BOINC/boincmgr: $(top_builddir)/stage/$(bindir)/boincmgr BOINC
	cp $(top_builddir)/stage/$(bindir)/boincmgr BOINC/boincmgr

BOINC/boincscr: $(top_builddir)/stage/$(bindir)/boincscr BOINC
	cp $(top_builddir)/stage/$(bindir)/boincscr BOINC/boincscr

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

BOINC/binstall.sh: BOINC Makefile
	echo 'cd BOINC &&\
	echo "cd \"$$PWD\" && exec ./boinc \$$@" > run_client &&\
	echo "cd \"$$PWD\" && exec ./boincmgr \$$@" > run_manager &&\
	chmod +x run_client &&\
	chmod +x run_manager &&\
	echo use $$PWD/run_manager to start BOINC' > BOINC/binstall.sh

BOINC:
	mkdir -p BOINC

clean:
	rm -rf BOINC sea.tar $(sea)

