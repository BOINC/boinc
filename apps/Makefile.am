## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = upper_case concat 1sec wrapper

LDADD = -L../lib -L../api -lboinc_api -lboinc $(PTHREAD_LIBS)

upper_case_SOURCES       = upper_case.C
concat_SOURCES           = concat.C
1sec_SOURCES             = 1sec.C
wrapper_SOURCES           = wrapper.C
