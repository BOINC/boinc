noinst_LTLIBRARIES =

AM_CPPFLAGS = -I$(top_srcdir)/include

# Symbol file for the master side
EXTRA_DIST = master.sym

if BUILD_MASTER

noinst_LTLIBRARIES += libdc-common-master.la
libdc_common_master_la_CPPFLAGS = $(AM_CPPFLAGS) $(GLIB_CFLAGS) -DMASTER
libdc_common_master_la_SOURCES = \
		cfg-master.c \
		logger.c \
		util.c
libdc_common_master_la_LIBADD = $(GLIB_LIBS)

endif

if BUILD_CLIENT

noinst_LTLIBRARIES += libdc-common-client.la
libdc_common_client_la_CPPFLAGS = $(AM_CPPFLAGS) -DCLIENT
libdc_common_client_la_SOURCES = \
		cfg-client.c \
		logger.c \
		util.c

endif
