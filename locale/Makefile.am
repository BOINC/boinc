## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

datadir = $(prefix)/share/locale 

locale_dirs = \
	ar \
	be \
	bg \
	ca \
	cs \
	cy \
	da \
	de \
	el \
	es \
	fa \
	fi \
	fr \
	gl \
	hr \
	hu \
	it \
	ja \
	ko \
	lt \
	lv \
	nb \
	nl \
	nn \
	pl \
	pt_BR \
	pt_PT \
	ro \
	ru \
	sk \
	sl \
	sv_SE \
	tr \
	uk \
	zh_CN \
	zh_TW

install-data-hook:
	mydir=$(DESTDIR)$(datadir) ;\
	for ldir in $(locale_dirs) ; do \
	  $(INSTALL) -d $$mydir/$$ldir ; \
	  if [ -f $$ldir/BOINC-Manager.mo ] ; then \
	    $(INSTALL_DATA) $$ldir/BOINC-Manager.mo $$mydir/$$ldir/BOINC-Manager.mo ; \
	  fi ;\
	  if [ -f $$ldir/BOINC-Client.mo ] ; then \
	    $(INSTALL_DATA) $$ldir/BOINC-Client.mo $$mydir/$$ldir/BOINC-Client.mo ; \
	  fi ;\
	done

