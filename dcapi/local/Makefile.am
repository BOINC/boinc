lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdc-local.la
endif

if BUILD_CLIENT
lib_LTLIBRARIES += libdc-client-local.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/common

libdc_local_la_SOURCES = \
		dc.c \
		rm.c \
		event.c
libdc_local_la_LIBADD = ../common/libdc-common.la

libdc_client_local_la_SOURCES = client.c
