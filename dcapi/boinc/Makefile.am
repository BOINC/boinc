lib_LTLIBRARIES =

AM_CPPFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS)

if BUILD_MASTER
lib_LTLIBRARIES += libdcapi-boinc.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdcapi-client-boinc.la
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = dcapi-boinc-client.pc dcapi-boinc-master.pc

AM_LDFLAGS = $(BOINC_LDFLAGS) $(GLIB_LIBS)

noinst_HEADERS = common_defs.h dc_boinc.h

libdcapi_boinc_la_SOURCES = \
		config.C \
		dc.c \
		db.C \
		events.C \
		result.c \
		wu.C
libdcapi_boinc_la_LIBADD = \
		../common/libdcapi-common-master.la \
		-lsched -lboinc -lcrypto -luuid -lm
libdcapi_boinc_la_CPPFLAGS = $(AM_CPPFLAGS) $(BOINC_SERVER_CPPFLAGS)
libdcapi_boinc_la_LDFLAGS = -version-info 1:1:0 -export-symbols $(top_srcdir)/common/master.sym

libdcapi_client_boinc_la_SOURCES = \
		client.C
libdcapi_client_boinc_la_LIBADD = \
		../common/libdcapi-common-client.la \
		$(BOINC_CLIENT_LIBS)
libdcapi_client_boinc_la_CPPFLAGS = $(AM_CPPFLAGS) $(BOINC_CPPFLAGS)
libdcapi_client_boinc_la_LDFLAGS = -static
