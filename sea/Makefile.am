EXTRA_DIST = make-sea.sh

vers = @BOINC_MAJOR_VERSION@.@BOINC_MINOR_VERSION@_@target@
sea = boinc_$(vers).sh

all: $(sea)

# WARNING: WHAT FOLLOWS IS 'FRAGILE'.  DO NOT REPLACE
# TABS WITH SPACES!

$(sea): sea.tar make-sea.sh Makefile
	./make-sea.sh sea.tar $(sea) BOINC/binstall.sh

sea.tar: BOINC/boinc BOINC/boincmgr BOINC/binstall.sh
	tar cf sea.tar BOINC/boinc BOINC/boincmgr BOINC/binstall.sh

BOINC/boinc: ../client/boinc BOINC
	cp ../client/boinc BOINC/boinc

BOINC/boincmgr: ../clientgui/boincmgr BOINC
	cp ../clientgui/boincmgr BOINC/boincmgr

BOINC/binstall.sh: BOINC Makefile
	echo 'cd BOINC &&\
	echo "cd \"`pwd`\" && ./boinc" > run_client &&\
	chmod +x run_client &&\
	echo now run `pwd`/run_client to run the client and `pwd`/boincmgr to run the GUI' > BOINC/binstall.sh

BOINC:
	mkdir -p BOINC

clean:
	rm -rf BOINC sea.tar $(sea)

