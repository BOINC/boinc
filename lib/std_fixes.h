// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_STD_FIXES_H
#define BOINC_STD_FIXES_H

#ifdef __cplusplus

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
#endif /* HAVE_STD_MIN */

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
#endif /* HAVE_STD_MAX */

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

#endif /* HAVE_STD_TRANSFORM */
#endif /* CONFIG_TEST */

#if defined(__cplusplus) && defined(LARGEFILE_BREAKS_CXX) && \
    (defined(_LARGE_FILES) || (_FILE_OFFSET_BITS==64)) \
  && !defined(_USING_FCGI_)

#include <stdio.h>
#include <fcntl.h>

#undef fopen
#undef freopen
#undef tmpfile
#undef fgetpos
#undef fsetpos
#undef fseeko
#undef ftello
#undef open
#undef creat

extern "C" {
FILE *fopen(const char *, const char *);
FILE *freopen(const char *, const char *, FILE *);
FILE *tmpfile(void);
int fgetpos(FILE *, fpos64_t *);
int fsetpos(FILE *, const fpos64_t *);
int fseeko(FILE *, const off64_t, int);
off64_t ftello(FILE *);
int open(const char *, int, mode_t mode=0);
int creat(const char *, mode_t mode);
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
inline int open(const char *filename, int flags, mode_t mode) { return open64(filename,flags,mode); }
inline int creat(const char *filename, mode_t mode) { return creat64(filename,mode); }

#endif
#endif /* __cplusplus */
#endif // BOINC_STD_FIXES_H

