ACLOCAL_AMFLAGS = -I .

AUTOMAKE_OPTIONS = 1.7

appdir = ${libdir}/boinc/apps/serialize-example
app_PROGRAMS =
app_DATA = dc-api.conf input.txt

EXTRA_DIST = \
	dc-api.conf \
	input.txt \
	serialize-example-client.xml.in \
	serialize-example-master.xml.in

if BUILD_CLIENT
app_PROGRAMS += serialize-example-client
app_DATA += serialize-example-client.xml
endif

if BUILD_MASTER
app_PROGRAMS += serialize-example-master
app_DATA += serialize-example-master.xml
endif

CLEANFILES = serialize-example-client.xml serialize-example-master.xml

serialize_example_client_SOURCES = client.c common.h
serialize_example_client_CPPFLAGS = $(DCAPI_CLIENT_CFLAGS)
serialize_example_client_LDADD = $(DCAPI_CLIENT_LIBS)

serialize_example_master_SOURCES = master.c common.h
serialize_example_master_CPPFLAGS = $(DCAPI_MASTER_CFLAGS)
serialize_example_master_LDADD = $(DCAPI_MASTER_LIBS)

serialize-example-client.xml: serialize-example-client.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@

serialize-example-master.xml: serialize-example-master.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@
