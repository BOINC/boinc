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

libdc_condor_la_SOURCES = condor_server.cc \
			  condor_common.c \
			  condor_wu.c
libdc_condor_la_LIBADD = ../common/libdc-common-master.la $(GLIB_LIBS)

libdc_client_condor_la_SOURCES = condor_client.c \
				 condor_common.c
libdc_client_condor_la_LIBADD = ../common/libdc-common-client.la

appdir		= $(libdir)/dcapi/condor
app_PROGRAMS	= tc ts

tc_SOURCES	= tc.c
tc_LDADD	= ../common/libdc-common-client.la libdc-client-condor.la

ts_SOURCES 	= ts.c
ts_LDADD	= ../common/libdc-common-master.la libdc-condor.la -luuid
