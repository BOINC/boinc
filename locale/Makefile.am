## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

datadir = $(prefix)/share/locale

locale_dirs = \
	ar \
	be \
	bg \
	ca \
	cs \
	da \
	de \
	el \
	es \
	fa \
	fa_IR \
	fi \
	fr \
	gl \
	he \
	hr \
	hu \
	it_IT \
	ja \
	ka \
	ko \
	lt \
	lv \
	ms \
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
	sv \
	tr \
	uk \
	zh_CN \
	zh_TW

install-data-hook:
	mydir=$(DESTDIR)$(datadir) ;\
	for ldir in $(locale_dirs) ; do \
	  $(INSTALL) -d $$mydir/$$ldir/LC_MESSAGES ; \
	  if [ -f $(top_srcdir)/locale/$$ldir/BOINC-Manager.mo ] ; then \
	    $(INSTALL_DATA) $(top_srcdir)/locale/$$ldir/BOINC-Manager.mo $$mydir/$$ldir/LC_MESSAGES/BOINC-Manager.mo ; \
	  fi ;\
	  if [ -f $(top_srcdir)/locale/$$ldir/BOINC-Client.mo ] ; then \
	    $(INSTALL_DATA) $(top_srcdir)/locale/$$ldir/BOINC-Client.mo $$mydir/$$ldir/LC_MESSAGES/BOINC-Client.mo ; \
	  fi ;\
	done
