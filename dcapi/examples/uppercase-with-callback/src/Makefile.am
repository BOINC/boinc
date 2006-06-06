appdir = ${libdir}/boinc/apps/uppercase-demo
app_PROGRAMS =
app_DATA =

if BUILD_CLIENT
app_PROGRAMS += uppercase-demo-client
app_DATA += uppercase-demo-client.xml
endif

if BUILD_MASTER
app_PROGRAMS += uppercase-demo-master
app_DATA += uppercase-demo-master.xml
endif

CLEANFILES = uppercase-demo-client.xml uppercase-demo-master.xml

uppercase_demo_client_SOURCES = client.c
uppercase_demo_client_CPPFLAGS = $(DCAPI_CLIENT_CFLAGS)
uppercase_demo_client_LDADD = $(DCAPI_CLIENT_LIBS)

uppercase_demo_master_SOURCES = master.c
uppercase_demo_master_CPPFLAGS = $(DCAPI_MASTER_CFLAGS)
uppercase_demo_master_LDADD = $(DCAPI_MASTER_LIBS)

uppercase-demo-client.xml: uppercase-demo-client.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@

uppercase-demo-master.xml: uppercase-demo-master.xml.in
	$(SED) -e "s,@PACKAGE_VERSION\@,$(PACKAGE_VERSION)," \
		-e "s,@appdir\@,$(appdir)," \
		-e "s,@target\@,$(target_triplet)," \
		$< > $@
