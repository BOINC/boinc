all: boinc_sea.sh

boinc_sea.sh: make-sea.sh sea.tar
	./make-sea.sh sea.tar boinc_sea.sh BOINC/binstall.sh

sea.tar: BOINC/boinc_client BOINC/boinc_gui BOINC/binstall.sh
	tar chf $@ $^

BOINC/boinc_client: ../client/boinc_client
	ln -s ../$< $@

BOINC/boinc_gui: ../clientgui/boinc_gui
	ln -s ../$< $@

clean:
	rm -f BOINC/boinc_* sea.tar boinc_sea.sh
