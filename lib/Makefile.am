## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = crypt_prog
EXTRA_PROGRAMS = md5_test shmem_test msg_test
noinst_LIBRARIES = libboinc.a 
#libboinc_graphics.a
EXTRA_DIST = result_state.h parse.h std_fixes.h util.h

# libboinc.a is for use by applications (not core client or server)

libboinc_a_SOURCES =				\
    ../api/boinc_api.C			\
    ../api/mfile.C				\
    app_ipc.C				\
	base64.C				\
    diagnostics.C           \
    exception.C             \
    filesys.C				\
    language.C				\
    md5.c					\
    md5_file.C				\
    parse.C					\
	shmem.C					\
    util.C

# libboinc_graphics_a_SOURCES =			\
#         ../api/graphics_api.C			\
#         ../api/graphics_data.C

md5_test_SOURCES = md5_test.C md5.c md5_file.C
shmem_test_SOURCES = shmem_test.C shmem.C
msg_test_SOURCES = msg_test.C msg_queue.C
crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES = $(LIBRSA)
crypt_prog_LDADD = $(RSA_LIBS)
