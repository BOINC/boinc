#ifndef _STD_FIXES_H_
#define _STD_FIXES_H_

#ifndef CONFIG_TEST
#ifndef HAVE_STD_MIN
namespace std {

#ifdef min
#undef min
#endif

template <typename T>
inline T min(const T &a, const T &b) {
	return ((a<b)?a:b);
}
}
#endif

#ifndef HAVE_STD_MAX
namespace std {
#ifdef max
#undef max
#endif

template <typename T>
inline T max(const T &a, const T &b) {
	return ((a>b)?a:b);
}

}
#endif

#ifndef HAVE_STD_TRANSFORM
#include <algorithm>
#include <iterator>

namespace std {
#ifdef transform
#undef transform
#endif


template <typename i_iterator, typename o_iterator, typename OP>
o_iterator transform(i_iterator first, i_iterator last, o_iterator res, OP op) {
	for (;first != last; first++) {
		*(res++)=op(*first);
	}
	return (res);
}

}

#endif
#endif

#if defined(LARGEFILE_BREAKS_CXX) && (defined(_LARGE_FILES) || (_FILE_OFFSET_BITS==64))

#include <stdio.h>

#undef fopen
#undef freopen
#undef tmpfile
#undef fgetpos
#undef fsetpos
#undef fseeko
#undef ftello

extern "C" {
FILE *fopen(const char *, const char *);
FILE *freopen(const char *, const char *, FILE *);
FILE *tmpfile(void);
int fgetpos(FILE *, fpos64_t *);
int fsetpos(FILE *, const fpos64_t *);
int fseeko(FILE *, const off64_t, int);
off64_t ftello(FILE *);
}

inline FILE *fopen(const char *path, const char *mode) { return fopen64(path,
    mode); }
inline FILE *freopen(const char *path, const char *mode, FILE *file) { return
  freopen64(path,mode,file); }
inline FILE *tmpfile(void) {return tmpfile64();}
inline int fgetpos(FILE *file, fpos64_t *pos) { return fgetpos64(file,pos); }
inline int fsetpos(FILE *file, const fpos64_t *pos) { return fsetpos64(file,pos); }
inline int fseeko(FILE *file, const off64_t pos, int whence) {return fseeko64(file,pos,whence);}
inline off64_t ftello(FILE *file) {return ftello64(file);}

#endif
#endif

