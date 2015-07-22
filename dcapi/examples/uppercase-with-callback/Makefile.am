ACLOCAL_AMFLAGS = -I .

AUTOMAKE_OPTIONS = 1.7

appdir = ${libdir}/boinc/apps/uppercase-example
app_PROGRAMS =
app_DATA = dc-api.conf input.txt

EXTRA_DIST = \
	dc-api.conf \
	input.txt \
	uppercase-example-client.xml.in \
	uppercase-example-master.xml.in

if BUILD_CLIENT
app_PROGRAMS += uppercase-example-client
app_DATA += uppercase-example-client.xml
endif

if BUILD_MASTER
app_PROGRAMS += uppercase-example-master
app_DATA += uppercase-example-master.xml
endif

CLEANFILES = uppercase-example-client.xml uppercase-example-master.xml

uppercase_example_client_SOURCES = client.c common.h
uppercase_example_client_CPPFLAGS = $(DCAPI_CLIENT_CFLAGS)
uppercase_example_client_LDADD = $(DCAPI_CLIENT_LIBS)

uppercase_example_master_SOURCES = master.c common.h
uppercase_example_master_CPPFLAGS = $(DCAPI_MASTER_CFLAGS)
uppercase_example_master_LDADD = $(DCAPI_MASTER_LIBS)

uppercase-example-client.xml: uppercase-example-client.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@

uppercase-example-master.xml: uppercase-example-master.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@
