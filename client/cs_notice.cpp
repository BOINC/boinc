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

#include <string>
#include <set>

#include "parse.h"
#include "url.h"

#include "client_state.h"
#include "cs_notice.h"

using std::vector;
using std::string;
using std::set;

NOTICES notices;
vector<RSS_FEED> rss_feeds;

// write notices newer than seqno as XML
//
void NOTICES::write(int seqno, MIOFILE& fout, bool public_only) {
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

void NOTICES::append(NOTICE& n) {
    notices.push_front(n);
}

void NOTICES::append_unique(NOTICE& n) {
    for (unsigned int i=0; i<notices.size(); i++) {
        NOTICE& n2 = notices[i];
        if (!strcmp(n.guid, n2.guid)) return;
    }
    append(n);
}

// called when a scheduler RPC finishes.
// Add or remove RSS feeds

// top-level poll.  See if need to start RSS fetch
//

// start an RSS fetch
//

// called when a fetch completes.
// Add new items to "notices", prune old items, 
// write disk file if got new items.
//

// called on startup.  Read disk files.
//
void init_rss_feeds() {
    for (unsigned int i=0; i<rss_feeds.size(); i++) {
        rss_feeds[i].read_feed_file();
    }
}

void RSS_FEED::feed_file_name(char* path) {
    char buf[256];
    escape_project_url(url, buf);
    sprintf(path, "feeds/%s", buf);
}

void RSS_FEED::read_feed_file() {
    char path[256];
    feed_file_name(path);
    FILE* f = fopen(path, "r");
    MIOFILE fin;
    fin.init_file(f);
    XML_PARSER xp(&fin);
    fclose(f);
}

// parse the descriptor in scheduler reply
//
int RSS_FEED::parse_desc(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/notice_feed")) return 0;
        if (xp.parse_str(tag, "url", url, sizeof(url))) continue;
        if (xp.parse_double(tag, "poll_interval", poll_interval)) continue;
    }
    return ERR_XML_PARSE;
}

void RSS_FEED::write(MIOFILE& fout) {
    fout.printf(
        "<rss_feed>\n"
        "   <url>%s</url>\n"
        "   <poll_interval>%f</poll_interval>\n",
        "</rss_feed>\n",
        url,
        poll_interval
    );
}

// parse the actual RSS feed
//
int RSS_FEED::parse_items(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/rss")) return 0;
        if (!strcmp(tag, "item")) {
            NOTICE n;
            int retval = n.parse_rss(xp);
            if (!retval) {
                if (append_seqno) {
                    notices.append(n);
                } else {
                    notices.append_unique(n);
                }
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

// parse the contents of the archive file
//
int RSS_FEED::parse_notices(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/notices")) return 0;
        if (!strcmp(tag, "notice")) {
            NOTICE n;
            int retval = n.parse(xp);
            if (!retval) {
                notices.append(n);
            }
        }
    }
    return ERR_XML_PARSE;
}

// parse notice feeds from scheduler reply
//
int parse_notice_feeds(MIOFILE& fin, vector<RSS_FEED>& feeds) {
    char tag[256];
    bool is_tag;
    XML_PARSER xp(&fin);
    int retval;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/notice_feeds")) return 0;
        if (!strcmp(tag, "notice_feed")) {
            RSS_FEED rf;
            retval = rf.parse_desc(xp);
            if (!retval) {
                feeds.push_back(rf);
            }
        }
    }
    return ERR_XML_PARSE;
}

// write a project's RSS feeds to the state file
//
void write_notice_feeds(MIOFILE& fout, vector<RSS_FEED>& feeds) {
    if (!feeds.size()) return;
    fout.printf("<rss_feeds>\n");
    for (unsigned int i=0; i<feeds.size(); i++) {
        feeds[i].write(fout);
    }
    fout.printf("</rss_feeds>\n");
}

// A feed has been updated.  Propagate to duplicates
//
void update_duplicates(RSS_FEED& rf, PROJECT* p0) {
    unsigned int i, j;
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p == p0) continue;
        for (j=0; j<p->proj_feeds.size(); j++) {
            if (!strcmp(rf.url, p->proj_feeds[i].url)) {
                p->proj_feeds[i] = rf;
                break;
            }
        }
    }
}

// the two feeds have same URL.  Do they differ in some other way?
//
static bool different(RSS_FEED& f1, RSS_FEED&f2) {
    if (f1.poll_interval != f2.poll_interval) return true;
    if (f1.append_seqno != f2.append_seqno) return true;
    return false;
}

// go through the set of all feeds,
// and if there are multiple that refer to the same URL,
// pick one as master and the others as duplicates
//
static void assign_masters() {
    set<string> urls;
    unsigned int i, j;

    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        for (j=0; j<p->proj_feeds.size(); j++) {
            RSS_FEED& rf = p->proj_feeds[j];
            if (urls.find(string(rf.url)) != urls.end()) {
                rf.duplicate = true;
            } else {
                urls.insert(string(rf.url));
                rf.duplicate = false;
            }
        }
    }
}

// A scheduler RPC returned a list (possibly empty) of feeds.
// Add new ones to the project's set,
// and remove ones from the project's set that aren't in the list.
//
void handle_notice_feeds(vector<RSS_FEED>& feeds, PROJECT* p) {
    unsigned int i, j;
    bool feed_set_changed = false;

    // mark current feeds as not found
    //
    for (i=0; i<p->proj_feeds.size(); i++) {
        p->proj_feeds[i].found = false;
    }

    for (i=0; i<feeds.size(); i++) {
        RSS_FEED& rf = feeds[i];
        bool present = false;
        for (j=0; j<p->proj_feeds.size(); j++) {
            RSS_FEED& rf2 = p->proj_feeds[j];
            if (!strcmp(rf.url, rf2.url)) {
                // update the permanent copy if needed
                if (different(rf, rf2)) {
                    rf2 = rf;
                    update_duplicates(rf2, p);
                }
                present = true;
                break;
            }
        }
        if (!present) {
            rf.found = true;
            p->proj_feeds.push_back(rf);
            feed_set_changed = true;
            update_duplicates(rf, p);
        }
    }

    // remove ones no longer present
    //
    vector<RSS_FEED>::iterator iter = p->proj_feeds.begin();
    while (iter != p->proj_feeds.end()) {
        RSS_FEED& rf = *iter;
        if (rf.found) {
            iter++;
        } else {
            // TODO: make sure this feed isn't in the middle of an RPC
            //
            iter = p->proj_feeds.erase(iter);
            feed_set_changed = true;
        }
    }

    // if anything was added or removed, pick masters
    //
    if (feed_set_changed) {
        assign_masters();
    }
}
