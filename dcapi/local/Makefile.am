lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdcapi-local.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdcapi-client-local.la
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = dcapi-local-client.pc dcapi-local-master.pc

AM_CPPFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS)

noinst_HEADERS = \
		local_common.h \
		local_master.h \
		local_result.h \
		local_utils.h \
		local_wu.h

libdcapi_local_la_SOURCES = \
		local_master.c \
		local_events.c \
		local_result.c \
		local_utils.c \
		local_common.c \
		local_wu.c
libdcapi_local_la_LIBADD = ../common/libdcapi-common-master.la $(GLIB_LIBS) -luuid
libdcapi_local_la_LDFLAGS = -version-info 1:0:0 -export-symbols $(top_srcdir)/common/master.sym

libdcapi_client_local_la_SOURCES = \
		local_client.c \
		local_utils.c \
		local_common.c
libdcapi_client_local_la_LIBADD = ../common/libdcapi-common-client.la
libdcapi_client_local_la_LDFLAGS = -static


#appdir		= $(libdir)/dcapi/local
#app_PROGRAMS	= lss lm

#lss_SOURCES	= ../condor/tss.c ../condor/tc.c
#lss_LDADD	= ../common/libdcapi-common-client.la libdcapi-client-local.la

#lm_SOURCES 	= ../condor/tm.c ../condor/tc.c
#lm_LDADD	= ../common/libdcapi-common-master.la libdcapi-local.la -L. -ldl -luuid -lstdc++

#dd: all install_local

#install_local:
#	ssh root@emma 'make -C /home/staff/drdani/prj/szdg/dcapi/trunk/local install'
