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

libdc_condor_la_SOURCES = 
libdc_condor_la_LIBADD = ../common/libdc-common-master.la $(GLIB_LIBS)

libdc_client_condor_la_SOURCES = 
libdc_client_condor_la_LIBADD = ../common/libdc-common-client.la
