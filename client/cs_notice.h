// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

#include <deque>

#include "miofile.h"
#include "notice.h"

extern std::deque<NOTICE> notices;

extern void write_notices(int seqno, MIOFILE&, bool public_only);

#if 0
// RSS_FEED represents an RSS feed that the client
// polls periodically and caches on disk

struct RSS_FEED {
    char url[256];
    std::vector<RSS_ITEM> items;

    int fetch_start();
    int fetch_complete();
};
#endif
