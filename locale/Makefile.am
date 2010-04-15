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

install-exec-hook:
	mydir=$(DESTDIR)$(datadir) ;\
	for ldir in $(locale_dirs) ; do \
	  $(INSTALL) -d $$mydir/$$ldir ; \
	  if [ -f $$ldir/BOINC-Manager.mo ] ; then \
	    $(INSTALL) $$ldir/BOINC-Manager.mo $$mydir/$$ldir/BOINC-Manager.mo ; \
	  fi ;\
	done

