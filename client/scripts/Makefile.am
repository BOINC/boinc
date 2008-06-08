## -*- mode: makefile; tab-width: 4 -*-

install-exec-hook:
	chmod +x boinc-client
	$(INSTALL) -d $(DESTDIR)/etc/init.d
	$(INSTALL) -b boinc-client $(DESTDIR)/etc/init.d/boinc-client
	if [ -d /etc/sysconfig ] ; then \
	  $(INSTALL) -d $(DESTDIR)/etc/sysconfig ; \
	  $(INSTALL) boinc-client.conf $(DESTDIR)/etc/sysconfig/boinc-client ; \
	elif [ -d /etc/default ] ; then \
	  $(INSTALL) -d $(DESTDIR)/etc/default ; \
	  $(INSTALL) boinc-client.conf $(DESTDIR)/etc/default/boinc-client ; \
	else \
	  $(INSTALL) -d $(DESTDIR)/etc ; \
	  $(INSTALL) boinc-client.conf $(DESTDIR)/etc/boinc-client.conf ; \
	fi

