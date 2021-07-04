## -*- mode: makefile; tab-width: 4 -*-

install-exec-hook:
	chmod +x boinc-client
	if [ -d /etc/init.d ] ; then \
		$(INSTALL) -d $(DESTDIR)$(sysconfdir)/init.d ; \
		$(INSTALL) -b boinc-client $(DESTDIR)$(sysconfdir)/init.d/boinc-client ; \
	fi
	if [ -d /usr/lib/systemd/system ] ; then \
		$(INSTALL) -d $(DESTDIR)/usr/lib/systemd/system/ ; \
		$(INSTALL_DATA) boinc-client.service $(DESTDIR)/usr/lib/systemd/system/boinc-client.service ; \
	elif [ -d /lib/systemd/system ] ; then \
		$(INSTALL) -d $(DESTDIR)/lib/systemd/system/ ; \
		$(INSTALL_DATA) boinc-client.service $(DESTDIR)/lib/systemd/system/boinc-client.service ; \
	fi
	if [ -d /etc/sysconfig ] ; then \
	  $(INSTALL) -d $(DESTDIR)$(sysconfdir)/sysconfig ; \
	  $(INSTALL_DATA) $(srcdir)/boinc-client.conf $(DESTDIR)$(sysconfdir)/sysconfig/boinc-client ; \
	elif [ -d /etc/default ] ; then \
	  $(INSTALL) -d $(DESTDIR)$(sysconfdir)/default ; \
	  $(INSTALL_DATA) $(srcdir)/boinc-client.conf $(DESTDIR)$(sysconfdir)/default/boinc-client ; \
	else \
	  $(INSTALL) -d $(DESTDIR)$(sysconfdir) ; \
	  $(INSTALL_DATA) $(srcdir)/boinc-client.conf $(DESTDIR)$(sysconfdir)/boinc-client.conf ; \
	fi

clean:
	rm -f boinc-client.service
	rm -f boinc-client
