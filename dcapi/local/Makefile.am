lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdc-local.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdc-client-local.la
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = dcapi-local-client.pc dcapi-local-master.pc

AM_CPPFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS)

noinst_HEADERS = common_defs.h dc_local.h

libdc_local_la_SOURCES = \
		dc.c \
		rm.c \
		events.c \
		result.c
libdc_local_la_LIBADD = ../common/libdc-common-master.la $(GLIB_LIBS)

libdc_client_local_la_SOURCES = client.c
libdc_client_local_la_LIBADD = ../common/libdc-common-client.la
