lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdc-condor.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdc-client-condor.la
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = dcapi-condor-client.pc dcapi-condor-master.pc

CONDOR_HOME	= /home/other/condor
AM_CPPFLAGS	= -I$(top_srcdir)/common $(GLIB_CFLAGS) \
		  -I$(CONDOR_HOME)/include
AM_LDFLAGS	= $(BOINC_LDFLAGS) $(GLIB_LIBS)
AM_CFLAGS	= -O0 -g
AM_CXXFLAGS	= -O0 -g

noinst_HEADERS	= condor_common.h \
		  condor_defs.h \
		  condor_master.h \
		  condor_wu.h \
		  condor_file.h \
		  condor_log.h \
		  condor_utils.h

libdc_condor_la_SOURCES = condor_master.c \
			  condor_common.c \
			  condor_file.c \
			  condor_wu.c \
			  condor_managewu.c \
			  condor_log.cc \
			  condor_file.c \
			  condor_utils.c

libdc_condor_la_LIBADD = ../common/libdc-common-master.la \
			 $(GLIB_LIBS) \
			 -l$(CONDOR_HOME)/lib/libcondorapi.a

libdc_client_condor_la_SOURCES = condor_slave.c \
				 condor_common.c \
				 condor_utils.c

libdc_client_condor_la_LIBADD = ../common/libdc-common-client.la

#appdir		= $(libdir)/dcapi/condor
#app_PROGRAMS	= ts tm
#
#ts_SOURCES	= ts.c
#ts_LDADD	= ../common/libdc-common-client.la \
#		  libdc-client-condor.la
#
#tm_SOURCES 	= tm.c
#tm_LDADD	= ../common/libdc-common-master.la \
#		  libdc-condor.la \
#		  -luuid -lstdc++


dd: all
	ssh root@emma 'make -C /home/local2/drdani/prj/szdg/dcapi/trunk/condor install'
	$(MAKE) -C /home/local2/drdani/prj/szdg/apps/uppercase-demo/trunk clean all
	cp /home/local2/drdani/prj/szdg/apps/uppercase-demo/trunk/src/uppercase-demo-client uppercase-demo
	cp /home/local2/drdani/prj/szdg/apps/uppercase-demo/trunk/src/uppercase-demo-master .