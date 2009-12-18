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

#include "cs_notice.h"

using std::deque;

deque<NOTICE> notices;

void write_notices(int seqno, MIOFILE& fout, bool public_only) {
    unsigned int i;
    fout.printf("<notices>\n");
    for (i=0; i<notices.size(); i++) {
        NOTICE& n = notices[i];
        if (n.seqno <= seqno) break;
        if (public_only && n.is_private) continue;
        n.write(fout);
    }
    fout.printf("</notices>\n");
}
