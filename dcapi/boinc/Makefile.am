lib_LTLIBRARIES = libdc-boinc.la

AM_CPPFLAGS = -I$(top_srcdir)/common $(BOINC_CPPFLAGS) $(MYSQL_CPPFLAGS)

libdc_boinc_la_SOURCES = \
		assimilator.C \
		assimilator.h \
		dc.c \
		result.c \
		result.h \
		validate_util.C \
		validate_util.h \
		wu.C \
		wu.h
libdc_boinc_la_LIBADD = \
		../common/libdc-common.la \
		$(BOINC_LIBS) \
		$(MYSQL_LIBS)
