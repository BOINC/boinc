vers = @BOINC_MAJOR_VERSION@.@BOINC_MINOR_VERSION@_@target@
sea = boinc_$(vers)_sea.sh

all: $(sea)

$(sea): sea.tar make-sea.sh Makefile
	./make-sea.sh $< $@ BOINC/binstall.sh

sea.tar: BOINC/boinc_client BOINC/boinc_gui BOINC/binstall.sh
	tar cf $@ $^

BOINC/boinc_client: ../client/boinc_client BOINC
	cp $< $@

BOINC/boinc_gui: ../clientgui/boinc_gui BOINC
	cp $< $@

BOINC/binstall.sh: BOINC
	echo 'cd BOINC &&\
	echo "cd \"`pwd`\" && ./boinc_client" > run_client &&\
	chmod +x run_client &&\
	echo now run `pwd`/run_client to run the client and `pwd`/boinc_gui to run the GUI' > $@

BOINC:
	mkdir -p BOINC

clean:
	rm -rf BOINC sea.tar $(sea)
