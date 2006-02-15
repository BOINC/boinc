lib_LTLIBRARIES =

if BUILD_MASTER
lib_LTLIBRARIES += libdc-local.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/common

libdc_local_la_SOURCES = \
		dc.c \
		rm.c
libdc_local_la_LIBADD = ../common/libdc-common.la
