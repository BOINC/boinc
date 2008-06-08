## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

datadir = $(prefix)/share/locale 

locale_dirs = \
        af_ZA \
	ar \
	be \
	bg \
	ca \
	cs \
	cs_CZ \
	da \
	de \
	el \
	en_US \
	es \
	eu \
	fi \
	fr \
	hr \
	hu \
	it \
	ja \
	ko_KR \
	lt \
	lv \
	nb \
	nl \
	pl \
	pt \
	pt_BR \
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
	  if [ -f $$ldir/BOINC-Manager.po ] ; then \
	    $(INSTALL) $$ldir/BOINC-Manager.po $$mydir/$$ldir/BOINC-Manager.po ; \
	  fi ;\
	done

