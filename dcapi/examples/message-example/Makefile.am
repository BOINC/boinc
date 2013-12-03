ACLOCAL_AMFLAGS = -I .

AUTOMAKE_OPTIONS = 1.7

appdir = ${libdir}/boinc/apps/message-example
app_PROGRAMS =
app_DATA = dc-api.conf input.txt

EXTRA_DIST = \
	dc-api.conf \
	input.txt \
	message-example-client.xml.in \
	message-example-master.xml.in

if BUILD_CLIENT
app_PROGRAMS += message-example-client
app_DATA += message-example-client.xml
endif

if BUILD_MASTER
app_PROGRAMS += message-example-master
app_DATA += message-example-master.xml
endif

CLEANFILES = message-example-client.xml message-example-master.xml

message_example_client_SOURCES = client.c common.h
message_example_client_CPPFLAGS = $(DCAPI_CLIENT_CFLAGS)
message_example_client_LDADD = $(DCAPI_CLIENT_LIBS)

message_example_master_SOURCES = master.c common.h
message_example_master_CPPFLAGS = $(DCAPI_MASTER_CFLAGS)
message_example_master_LDADD = $(DCAPI_MASTER_LIBS)

message-example-client.xml: message-example-client.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@

message-example-master.xml: message-example-master.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@
