## $Id$

SUBDIRS = RSAEuro db lib api apps client sched

EXTRA_DIST =								\
        mac_build/boinc.pbproj/*.pbxproj mac_build/*.lproj/*.nib/*.*	\
        mac_build/*.lproj/*.strings					\
        win_build/*.* win_build/*/*.dsp win_build/*/*.rct		\
        doc/*.*								\
        html_ops/*.*							\
        html_user/*.*							\
        stripchart/* stripchart/samples/*				\
        test/*.xml test/*.php test/*wu test/*result test/*output	\
        test/*input test/*.inc						\
        INSTALL LICENSE

NOT_IN_CLIENT_TARBALL = db doc html_ops html_user lib stripchart sched test tools api

distdir_client = $(PACKAGE)_client-$(VERSION)

am__remove_distdir_client = \
  { test ! -d $(distdir_client) \
    || { find $(distdir_client) -type d ! -perm -200 -exec chmod u+w {} ';' \
         && rm -fr $(distdir_client); }; }

distdir-client: distdir
	mv $(distdir) $(distdir_client)
	cd $(distdir_client)
	rm -r $(NOT_IN_CLIENT_TARBALL)
	cd ..

client-dist-gzip: distdir-client-client
	$(AMTAR) chof - $(distdir_client) | GZIP=$(GZIP_ENV) gzip -c >$(distdir_client).tar.gz
	$(am__remove_distdir_client)

client-dist-bzip2: distdir-client-client
	$(AMTAR) chof - $(distdir_client) | bzip2 -9 -c >$(distdir_client).tar.bz2
	$(am__remove_distdir_client)

client-dist-zip: distdir-client-client
	-rm -f $(distdir_client).zip
	zip -rq $(distdir_client).zip $(distdir_client)
	$(am__remove_distdir_client)

dist-client dist-client-all: distdir-client
	$(AMTAR) chof - $(distdir_client) | GZIP=$(GZIP_ENV) gzip -c >$(distdir_client).tar.gz
	$(AMTAR) chof - $(distdir_client) | bzip2 -9 -c >$(distdir_client).tar.bz2
	-rm -f $(distdir_client).zip
	zip -rq $(distdir_client).zip $(distdir_client)
	$(am__remove_distdir_client)

# dist-all defined through automake makes the normal non-client distribution.

dist-all: dist-client-all

.PHONY: distdir-client client-dist-gzip client-dist-gzip2 client-dist-zip \
	dist-client dist-client-all
