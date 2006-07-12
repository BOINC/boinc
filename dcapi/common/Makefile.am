noinst_LTLIBRARIES = libdc-common-master.la libdc-common-client.la

AM_CPPFLAGS = -I$(top_srcdir)/include

libdc_common_master_la_CPPFLAGS = $(AM_CPPFLAGS) $(GLIB_CFLAGS) -DMASTER
libdc_common_master_la_SOURCES = \
		cfg-master.c \
		logger.c \
		util.c
libdc_common_master_la_LIBADD = $(GLIB_LIBS)

libdc_common_client_la_CPPFLAGS = $(AM_CPPFLAGS) -DCLIENT
libdc_common_client_la_SOURCES = \
		cfg-client.c \
		logger.c \
		util.c
