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

#include "error_numbers.h"
#include "notice.h"

int NOTICE::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/notice")) return 0;
        if (xp.parse_int(tag, "seqno", seqno)) continue;
        if (xp.parse_str(tag, "title", title, sizeof(title))) continue;
        if (xp.parse_string(tag, "description", description)) continue;
        if (xp.parse_double(tag, "create_time", create_time)) continue;
        if (xp.parse_double(tag, "arrival_time", arrival_time)) continue;
        if (xp.parse_bool(tag, "is_private", is_private)) continue;
        if (xp.parse_str(tag, "category", category, sizeof(category))) continue;
        if (xp.parse_str(tag, "url", url, sizeof(url))) continue;
    }
    return ERR_XML_PARSE;
}

void NOTICE::write(MIOFILE& f) {
    f.printf(
        "<notice>\n"
        "   <seqno>%d</seqno>\n"
        "   <title>%s</title>\n"
        "   <description>%s</description>\n"
        "   <create_time>%f</create_time>\n"
        "   <arrival_time>%f</arrival_time>\n"
        "   <is_private>%d</is_private>\n"
        "   <category>%s</category>\n"
        "   <url>%s</url>\n"
        "</notice>\n",
        seqno,
        title,
        description,
        create_time,
        arrival_time,
        is_private?1:0,
        category,
        url
    );
}
