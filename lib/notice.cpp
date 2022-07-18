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

#if defined(_WIN32)
#include "boinc_win.h"
#endif

#include "error_numbers.h"
#include "str_replace.h"

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
    clear();
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/notice")) {
            if (strcasestr(description.c_str(), "youtube.com")) {
                is_youtube_video = true;
            }
            return 0;
        }
        if (xp.parse_int("seqno", seqno)) continue;
        if (xp.parse_str("title", title, sizeof(title))) continue;
        if (xp.parse_string("description", description)) continue;
        if (xp.parse_double("create_time", create_time)) continue;
        if (xp.parse_double("arrival_time", arrival_time)) continue;
        if (xp.parse_bool("is_private", is_private)) continue;
        if (xp.parse_str("category", category, sizeof(category))) continue;
        if (xp.parse_str("link", link, sizeof(link))) continue;
        if (xp.parse_str("project_name", project_name, sizeof(project_name))) continue;
        if (xp.parse_str("guid", guid, sizeof(guid))) continue;
        if (xp.parse_str("feed_url", feed_url, sizeof(feed_url))) continue;
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
        "   <project_name>%s</project_name>\n"
        "   <category>%s</category>\n"
        "   <link><![CDATA[%s]]></link>\n",
        title,
        description.c_str(),
        create_time,
        arrival_time,
        is_private?1:0,
        project_name,
        category,
        link
    );
    if (for_gui) {
        f.printf(
            "   <seqno>%d</seqno>\n", seqno
        );
    } else {
        f.printf(
            "   <guid>%s</guid>\n", guid
        );
    }
    f.printf(
        "</notice>\n"
     );
}


void NOTICE::clear() {
    seqno = 0;
    safe_strcpy(title, "");
    description = "";
    create_time = 0;
    arrival_time = 0;
    is_private = false;
    is_youtube_video = false;
    safe_strcpy(category, "");
    safe_strcpy(link, "");
    safe_strcpy(project_name, "");
    safe_strcpy(guid, "");
    safe_strcpy(feed_url, "");
    keep = false;
}

