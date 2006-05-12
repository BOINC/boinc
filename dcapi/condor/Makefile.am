lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdc-condor.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdc-client-condor.la
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = dcapi-condor-client.pc dcapi-condor-master.pc

AM_CPPFLAGS = -I$(top_srcdir)/common $(GLIB_CFLAGS)
AM_LDFLAGS = $(BOINC_LDFLAGS) $(GLIB_LIBS)

noinst_HEADERS	= condor_common.h condor_defs.h condor_master.h condor_wu.h

libdc_condor_la_SOURCES = condor_master.cc \
			  condor_common.c \
			  condor_wu.c
libdc_condor_la_LIBADD = ../common/libdc-common-master.la \
			 $(GLIB_LIBS)

libdc_client_condor_la_SOURCES = condor_slave.c \
				 condor_common.c
libdc_client_condor_la_LIBADD = ../common/libdc-common-client.la

appdir		= $(libdir)/dcapi/condor
app_PROGRAMS	= ts tm

ts_SOURCES	= ts.c
ts_LDADD	= ../common/libdc-common-client.la \
		  libdc-client-condor.la

tm_SOURCES 	= tm.c
tm_LDADD	= ../common/libdc-common-master.la \
		  libdc-condor.la \
		  -luuid -lstdc++
