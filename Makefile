# Generated automatically from Makefile.in by configure.
all:
	cd db; make; cd ..; \
	cd lib; make; cd ..; \
	cd api; make; cd ..; \
	cd apps; make; cd ..; \
	cd client; make; cd ..; \
	cd sched; make; cd ..; \
	cd tools; make; cd ..;

clean:
	rm -f boinc.tar.gz config.cache; \
	cd lib; make clean; cd ..; \
	cd api; make clean; cd ..; \
	cd apps; make clean; cd ..; \
	cd client; make clean; cd ..; \
	cd sched; make clean; cd ..; \
	cd tools; make clean; cd ..;

tar: clean
	tar cf boinc.tar * ; gzip boinc.tar
