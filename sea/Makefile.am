EXTRA_DIST = make-sea.sh

vers = @BOINC_MAJOR_VERSION@.@BOINC_MINOR_VERSION@_@target@
sea = boinc_$(vers).sh
tarfiles = BOINC/boinc BOINC/boincmgr BOINC/boinc_cmd BOINC/binstall.sh\
	BOINC/boincmgr.8x8.png BOINC/boincmgr.16x16.png BOINC/boincmgr.32x32.png

all: $(sea)

# WARNING: WHAT FOLLOWS IS 'FRAGILE'.  DO NOT REPLACE
# TABS WITH SPACES!

$(sea): sea.tar make-sea.sh Makefile
	./make-sea.sh sea.tar $(sea) BOINC/binstall.sh

sea.tar: $(tarfiles)
	tar cf sea.tar $(tarfiles)

BOINC/boinc: ../client/boinc BOINC
	cp ../client/boinc BOINC/boinc

BOINC/boincmgr: ../clientgui/boincmgr BOINC
	cp ../clientgui/boincmgr BOINC/boincmgr

BOINC/boinc_cmd: ../lib/boinc_cmd
	cp ../lib/boinc_cmd BOINC/boinc_cmd

BOINC/boincmgr.16x16.png: boincmgr.16x16.png
	cp boincmgr.16x16.png BOINC/boincmgr.16x16.png
BOINC/boincmgr.32x32.png: boincmgr.32x32.png
	cp boincmgr.32x32.png BOINC/boincmgr.32x32.png
BOINC/boincmgr.8x8.png: boincmgr.8x8.png
	cp boincmgr.8x8.png BOINC/boincmgr.8x8.png

BOINC/binstall.sh: BOINC Makefile
	echo 'cd BOINC &&\
	echo "cd \"$$PWD\" && exec ./boinc \$$@" > run_client &&\
	chmod +x run_client &&\
	echo now run $$PWD/run_client to run the client and $$PWD/boincmgr to run the GUI' > BOINC/binstall.sh

BOINC:
	mkdir -p BOINC

clean:
	rm -rf BOINC sea.tar $(sea)

