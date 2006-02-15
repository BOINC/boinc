lib_LTLIBRARIES =
AM_CPPFLAGS = -I$(top_srcdir)/common

if BUILD_MASTER
lib_LTLIBRARIES += libdc-boinc.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdc-client-boinc.la
endif

AM_LDFLAGS = $(BOINC_LDFLAGS)

libdc_boinc_la_SOURCES = \
		assimilator.C \
		assimilator.h \
		dc.c \
		result.c \
		result.h \
		validate_util.C \
		validate_util.h \
		wu.C \
		wu.h
libdc_boinc_la_LIBADD = \
		../common/libdc-common.la \
		$(BOINC_SERVER_LIBS)
libdc_boinc_la_CPPFLAGS = $(BOINC_SERVER_CPPFLAGS)

libdc_client_boinc_la_SOURCES = \
		dc_client.c
libdc_client_boinc_la_LIBADD = \
		$(BOINC_CLIENT_LIBS)
libdc_client_boinc_la_CPPFLAGS = $(BOINC_CPPFLAGS)
