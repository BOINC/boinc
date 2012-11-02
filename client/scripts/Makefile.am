## -*- mode: makefile; tab-width: 4 -*-

install-exec-hook:
	chmod +x boinc-client
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/init.d
	$(INSTALL) -b $(srcdir)/boinc-client $(DESTDIR)$(sysconfdir)/init.d/boinc-client
	if [ -d /etc/sysconfig ] ; then \
	  $(INSTALL) -d $(DESTDIR)$(sysconfdir)/sysconfig ; \
	  $(INSTALL) $(srcdir)/boinc-client.conf $(DESTDIR)$(sysconfdir)/sysconfig/boinc-client ; \
	elif [ -d /etc/default ] ; then \
	  $(INSTALL) -d $(DESTDIR)$(sysconfdir)/default ; \
	  $(INSTALL) $(srcdir)/boinc-client.conf $(DESTDIR)$(sysconfdir)/default/boinc-client ; \
	else \
	  $(INSTALL) -d $(DESTDIR)$(sysconfdir) ; \
	  $(INSTALL) $(srcdir)/boinc-client.conf $(DESTDIR)$(sysconfdir)/boinc-client.conf ; \
	fi

