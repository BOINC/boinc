#include <string.h>
#include <stdarg.h>

#include "error_numbers.h"
#include "miofile.h"

MIOFILE::MIOFILE() {
    mf = 0;
    f = 0;
    buf = 0;
}

MIOFILE::~MIOFILE() {
}

void MIOFILE::init_mfile(MFILE* _mf) {
    mf = _mf;
}

void MIOFILE::init_file(FILE* _f) {
    f = _f;
}

void MIOFILE::init_buf(char* _buf) {
    buf = _buf;
}

int MIOFILE::printf(const char* format, ...) {
    int retval;

    va_list ap;
    va_start(ap, format);
    if (mf) {
        retval = mf->vprintf(format, ap);
    } else {
        retval = vfprintf(f, format, ap);
    }
    va_end(ap);
    return retval;
}

char* MIOFILE::fgets(char* dst, int len) {
    if (f) {
        return ::fgets(dst, len, f);
    }
    char* q = strchr(buf, '\n');
    if (!q) return 0;

    q++;
    int n = q - buf;
    if (n > len-1) n = len-1;
    memcpy(dst, buf, n);
    dst[n] = 0;

    buf = q;
    return dst;
}

// copy from a file to static buffer
//
int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len) {
    char buf[256];

    strcpy(p, "");
    while (in.fgets(buf, 256)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        strcat(p, buf);
    }
    fprintf(stderr, "copy_element_contents(): no end tag\n");
    return ERR_XML_PARSE;
}

int copy_element_contents(MIOFILE& in, const char* end_tag, string& str) {
    char buf[256];

    str = "";
    while (in.fgets(buf, 256)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        str += buf;
    }
    fprintf(stderr, "copy_element_contents(): no end tag\n");
    return ERR_XML_PARSE;
}
