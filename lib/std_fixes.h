#ifndef _STD_FIXES_H_
#define _STD_FIXES_H_

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