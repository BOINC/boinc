all: boinc_sea.sh

boinc_sea.sh: make-sea.sh sea.tar
	sh make-sea.sh sea.tar boinc_sea.sh BOINC/binstall.sh

sea.tar: BOINC/boinc_client BOINC/boinc_gui BOINC/binstall.sh
	tar cf $@ $^

BOINC/boinc_client: ../client/boinc_client
	cp $< $@

BOINC/boinc_gui: ../clientgui/boinc_gui
	cp $< $@

clean:
	rm -f BOINC/boinc_* sea.tar boinc_sea.sh
