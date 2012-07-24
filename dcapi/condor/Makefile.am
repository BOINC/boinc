lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdcapi-condor.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdcapi-client-condor.la
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = dcapi-condor-client.pc dcapi-condor-master.pc

AM_CPPFLAGS	= -I$(top_srcdir)/include $(GLIB_CFLAGS) $(CONDOR_CPPFLAGS)
AM_LDFLAGS	= $(GLIB_LIBS) $(CONDOR_LDFLAGS)

noinst_HEADERS	= condor_common.h \
		  condor_defs.h \
		  condor_master.h \
		  condor_wu.h \
		  condor_log.h \
		  condor_utils.h \
		  condor_result.h \
		  condor_slave.h \
		  condor_event.h

libdcapi_condor_la_SOURCES = condor_master.c \
			  condor_common.c \
			  condor_wu.c \
			  condor_managewu.c \
			  condor_log.cc \
			  condor_utils.c \
			  condor_result.c \
			  condor_event.c

libdcapi_condor_la_LIBADD = ../common/libdcapi-common-master.la \
			 $(GLIB_LIBS) \
			 -lcondorapi

libdcapi_client_condor_la_SOURCES = condor_slave.c \
				 condor_common.c \
				 condor_utils.c

libdcapi_client_condor_la_LIBADD = ../common/libdcapi-common-client.la
libdcapi_client_condor_la_LDFLAGS = -static

#appdir		= $(libdir)/dcapi/condor
#app_PROGRAMS	= tss tm

#tss_SOURCES	= tss.c tc.c
#tss_LDADD	= ../common/libdcapi-common-client.la libdcapi-client-condor.la

#tm_SOURCES 	= tm.c tc.c
#tm_LDADD	= ../common/libdcapi-common-master.la libdcapi-condor.la -L. -ldl -luuid -lstdc++

dd: all
	ssh root@emma 'make -C /home/staff/drdani/prj/szdg/dcapi/trunk/condor install'
	$(MAKE) -C ../../../apps/uppercase-demo/trunk clean all
	cp ../../../apps/uppercase-demo/trunk/src/uppercase-demo-client uppercase-demo
	cp ../../../apps/uppercase-demo/trunk/src/uppercase-demo-master .
