## -*- mode: makefile; tab-width: 4 -*-
EXTRA_DIST = make-sea.sh

vers = \
  @BOINC_MAJOR_VERSION@.@BOINC_MINOR_VERSION@.@BOINC_RELEASE@_@boinc_platform@
sea = boinc_$(vers).sh
tarfiles = BOINC/boinc BOINC/boincmgr BOINC/boinc_cmd BOINC/binstall.sh\
	BOINC/boincmgr.16x16.png BOINC/boincmgr.32x32.png BOINC/boincmgr.48x48.png \
    BOINC/ca-bundle.crt

sea_debug = boinc_$(vers)_debug.sh
tarfiles_debug = BOINC_debug/boinc.unmodified BOINC_debug/boincmgr.unmodified\
        BOINC_debug/boinccmd.unmodified BOINC_debug/binstall.sh\
	BOINC_debug/boincmgr.16x16.png BOINC_debug/boincmgr.32x32.png\
	BOINC_debug/boincmgr.48x48.png \
    BOINC_debug/ca-bundle.crt

all: $(sea) $(sea_debug)

# WARNING: WHAT FOLLOWS IS 'FRAGILE'.  DO NOT REPLACE
# TABS WITH SPACES!

$(sea): sea.tar make-sea.sh Makefile
	./make-sea.sh sea.tar $(sea) BOINC/binstall.sh

$(sea_debug): sea_debug.tar make-sea.sh Makefile
	./make-sea.sh sea_debug.tar $(sea_debug) BOINC_debug/binstall.sh

sea.tar: $(tarfiles)
	./make-tar.sh

sea_debug.tar: $(tarfiles_debug)
	./make-tar_debug.sh

BOINC/boinc: ../client/boinc BOINC
	cp ../client/boinc BOINC/boinc

BOINC/boincmgr: ../clientgui/boincmgr BOINC
	cp ../clientgui/boincmgr BOINC/boincmgr

BOINC/boinc_cmd: ../lib/boinc_cmd  BOINC
	cp ../lib/boinc_cmd BOINC/boinc_cmd

BOINC/boincmgr.16x16.png: boincmgr.16x16.png
	cp boincmgr.16x16.png BOINC/boincmgr.16x16.png
BOINC/boincmgr.32x32.png: boincmgr.32x32.png
	cp boincmgr.32x32.png BOINC/boincmgr.32x32.png
BOINC/boincmgr.48x48.png: boincmgr.48x48.png
	cp boincmgr.48x48.png BOINC/boincmgr.48x48.png
BOINC/ca-bundle.crt: ../curl/ca-bundle.crt
	cp ../curl/ca-bundle.crt BOINC/ca-bundle.crt

BOINC_debug/boinc.unmodified: ../client/boinc.unmodified BOINC_debug
	cp ../client/boinc.unmodified BOINC_debug/boinc.unmodified

BOINC_debug/boincmgr.unmodified: ../clientgui/boincmgr.unmodified BOINC_debug
	cp ../clientgui/boincmgr.unmodified BOINC_debug/boincmgr.unmodified

BOINC_debug/boinccmd.unmodified: ../lib/boinccmd.unmodified BOINC_debug
	cp ../lib/boinccmd.unmodified BOINC_debug/boinccmd.unmodified

BOINC_debug/boincmgr.16x16.png: boincmgr.16x16.png
	cp boincmgr.16x16.png BOINC_debug/boincmgr.16x16.png
BOINC_debug/boincmgr.32x32.png: boincmgr.32x32.png
	cp boincmgr.32x32.png BOINC_debug/boincmgr.32x32.png
BOINC_debug/boincmgr.48x48.png: boincmgr.48x48.png
	cp boincmgr.48x48.png BOINC_debug/boincmgr.48x48.png
BOINC_debug/ca-bundle.crt: ../curl/ca-bundle.crt
	cp ../curl/ca-bundle.crt BOINC_debug/ca-bundle.crt

BOINC/binstall.sh: BOINC Makefile
	echo 'cd BOINC &&\
	echo "cd \"$$PWD\" && exec ./boinc \$$@" > run_client &&\
	echo "cd \"$$PWD\" && exec ./boincmgr \$$@" > run_manager &&\
	chmod +x run_client &&\
	chmod +x run_manager &&\
	echo use $$PWD/run_manager to start BOINC' > BOINC/binstall.sh

BOINC_debug/binstall.sh: BOINC_debug Makefile
	echo 'cd BOINC_debug &&\
	echo "cd \"$$PWD\" && exec ./boinc.unmodified \$$@" > run_client &&\
	echo "cd \"$$PWD\" && exec ./boincmgr.unmodified \$$@" > run_manager &&\
	chmod +x run_client &&\
	chmod +x run_manager &&\
	echo use $$PWD/run_manager to start BOINC' > BOINC_debug/binstall.sh

BOINC:
	mkdir -p BOINC

BOINC_debug:
	mkdir -p BOINC_debug

clean:
	rm -rf BOINC sea.tar $(sea)
	rm -rf BOINC_debug sea_debug.tar $(sea_debug)

