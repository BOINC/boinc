## $Id$

SUBDIRS = RSAEuro db lib api apps client sched

###########################################################################
# The rest of the rules below deal with making .tar.gz, .tar.bz2, and .zip
# files. There are two sets of distributions: full and client.  full is built
# by "make dist" and client is built by "make dist-client". The
# NOT_IN_CLIENT_TARBALL variable contains sources that are not in the client
# distribution.
#

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =								\
	mac_build win_build						\
	doc html_ops html_user						\
	stripchart							\
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
	rm -fr $(distdir_client)
	mv $(distdir) $(distdir_client)
	cd $(distdir_client) && rm -r $(NOT_IN_CLIENT_TARBALL) && cd ..

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

dist-hook:
	rm -rf `find $(distdir) -name CVS`

.PHONY: distdir-client client-dist-gzip client-dist-gzip2 client-dist-zip \
	dist-client dist-client-all
