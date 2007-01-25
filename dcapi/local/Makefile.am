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

noinst_HEADERS = \
		local_common.h \
		local_master.h \
		local_utils.h

libdc_local_la_SOURCES = \
		local_master.c \
		local_events.c \
		local_result.c \
		local_utils.c \
		local_common.c \
		local_wu.c
libdc_local_la_LIBADD = ../common/libdc-common-master.la $(GLIB_LIBS)

libdc_client_local_la_SOURCES = \
		local_client.c \
		local_utils.c \
		local_common.c
libdc_client_local_la_LIBADD = ../common/libdc-common-client.la

#appdir		= $(libdir)/dcapi/local
#app_PROGRAMS	= lss lm

#lss_SOURCES	= ../condor/tss.c ../condor/tc.c
#lss_LDADD	= ../common/libdc-common-client.la libdc-client-local.la

#lm_SOURCES 	= ../condor/tm.c ../condor/tc.c
#lm_LDADD	= ../common/libdc-common-master.la libdc-local.la -L. -ldl -luuid -lstdc++

#dd: all install_local

#install_local:
#	ssh root@emma 'make -C /home/staff/drdani/prj/szdg/dcapi/trunk/local install'
