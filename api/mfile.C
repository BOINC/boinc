#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "boinc_api.h"

int MFILE::open(char* path, char* mode) {
    buf = 0;
    len = 0;
    f = fopen(path, mode);
    if (!f) return -1;
    return 0;
}

int MFILE::printf(char* format, ...) {
    va_list ap;
    char buf2[4096];
    int n, k;

    va_start(ap, format);
    k = vsprintf(buf2, format, ap);
    va_end(ap);
    n = strlen(buf2);
    buf = (char*)realloc(buf, len+n);
    strncpy(buf+len, buf2, n);
    len += n;
    return k;
}

size_t MFILE::write(const void *ptr, size_t size, size_t nitems) {
    buf = (char *)realloc( buf, len+(size*nitems) );
    memcpy( buf+len, ptr, size*nitems );
    len += size*nitems;
    return nitems;
}

int MFILE::_putchar(char c) {
    buf = (char*)realloc(buf, len+1);
    buf[len] = c;
    len++;
    return c;
}

int MFILE::puts(char* p) {
    int n = strlen(p);
    buf = (char*)realloc(buf, len+n);
    strncpy(buf+len, p, n);
    len += n;
    return 0;
}

int MFILE::close() {
    fwrite(buf, 1, len, f);
    free(buf);
    buf = 0;
    return fclose(f);
}

int MFILE::flush() {
    fwrite(buf, 1, len, f);
    len = 0;
    return fflush(f);
}
