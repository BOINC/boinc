## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = crypt_prog
EXTRA_PROGRAMS = md5_test shmem_test msg_test synch_test
noinst_LIBRARIES = libboinc.a 
#libboinc_graphics.a
EXTRA_DIST = result_state.h

libboinc_a_SOURCES =				\
        ../api/boinc_api.C			\
        ../api/mfile.C				\
        app_ipc.C				\
        parse.C					\
	base64.C				\
	xml_util.C				\
        filesys.C				\
        language.C				\
        util.C					\
        md5.c					\
        md5_file.C				\
	shmem.C					\
	app_ipc.h				\
	crypt.h					\
	error_numbers.h				\
	filesys.h				\
	language.h				\
	md5.h					\
	md5_file.h				\
	msg_queue.h				\
	parse.h					\
	xml_util.h				\
	shmem.h					\
	synch.h					\
	util.h

# libboinc_graphics_a_SOURCES =			\
#         ../api/graphics_api.C			\
#         ../api/graphics_data.C

md5_test_SOURCES = md5_test.C md5.c md5_file.C
shmem_test_SOURCES = shmem_test.C shmem.C
msg_test_SOURCES = msg_test.C msg_queue.C
synch_test_SOURCES = synch_test.C synch.C
crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES = $(LIBRSA)
crypt_prog_LDADD = $(RSA_LIBS)
