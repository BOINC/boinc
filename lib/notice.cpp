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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif

#include "error_numbers.h"
#include "notice.h"


NOTICE::NOTICE() {
    clear();
}


NOTICE::~NOTICE() {
    clear();
}


// This is to parse our own XML.
// parse_rss() parses an RSS feed item.
//
int NOTICE::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

//    memset(this, 0, sizeof(*this));
    clear();
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/notice")) {
            return 0;
        }
        if (xp.parse_int(tag, "seqno", seqno)) continue;
        if (xp.parse_str(tag, "title", title, sizeof(title))) continue;
        if (xp.parse_string(tag, "description", description)) continue;
        if (xp.parse_double(tag, "create_time", create_time)) continue;
        if (xp.parse_double(tag, "arrival_time", arrival_time)) continue;
        if (xp.parse_bool(tag, "is_private", is_private)) continue;
        if (xp.parse_str(tag, "category", category, sizeof(category))) continue;
        if (xp.parse_str(tag, "link", link, sizeof(link))) continue;
        if (xp.parse_str(tag, "project_name", project_name, sizeof(project_name))) continue;
        if (xp.parse_str(tag, "guid", guid, sizeof(guid))) continue;
        if (xp.parse_str(tag, "feed_url", feed_url, sizeof(feed_url))) continue;
    }
    return ERR_XML_PARSE;
}


void NOTICE::write(MIOFILE& f, bool for_gui) {
    f.printf(
        "<notice>\n"
        "   <title>%s</title>\n"
        "   <description><![CDATA[\n%s\n]]></description>\n"
        "   <create_time>%f</create_time>\n"
        "   <arrival_time>%f</arrival_time>\n"
        "   <is_private>%d</is_private>\n"
        "   <project_name>%s</project_name>"
        "   <category>%s</category>\n"
        "   <link>%s</link>\n",
        title,
        description.c_str(),
        create_time,
        arrival_time,
        is_private?1:0,
        project_name,
        category,
        link
    );
    if (!for_gui) {
        f.printf(
            "   <guid>%s</guid>\n", guid
        );
    } else {
        f.printf(
            "   <seqno>%d</seqno>\n", seqno
        );
    }
    f.printf(
        "</notice>\n"
     );
}


void NOTICE::clear() {
    seqno = 0;
    strcpy(title, "");
    description = "";
    create_time = 0;
    arrival_time = 0;
    is_private = 0;
    strcpy(category, "");
    strcpy(link, "");
    strcpy(project_name, "");
    strcpy(guid, "");
    strcpy(feed_url, "");
}

