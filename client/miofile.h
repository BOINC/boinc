#ifndef _MIOFILE_
#define _MIOFILE_

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <string>
using namespace std;
#endif

#include "mfile.h"

// MIOFILE lets you do formatted I/O to either a FILE or a memory buffer,
// depending on how you initialize it.
//
// output:
//  init_file(): output goes to the FILE* that you specify
//  init_mfile(): output goes to an MFILE (i.e. a memory buffer);
//     you can call MFILE::get_buf() to get the results
// input:
//  init_file(): input comes from the FILE* that you specify
//  init_buf(): input comes from the buffer you specify.
//   This string is not modified.
//
// Why is this here?  Because on Windows (9x, maybe all)
// you can't do fdopen() on a socket.

class MIOFILE {
public:
    MIOFILE();
    ~MIOFILE();
    void init_mfile(MFILE*);
    void init_file(FILE*);
    void init_buf(char*);
    int printf(const char* format, ...);
    char* fgets(char*, int);
private:
    MFILE* mf;
    FILE* f;
    char* buf;
};

extern int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len);
extern int copy_element_contents(MIOFILE& in, const char* end_tag, string&);

#endif
