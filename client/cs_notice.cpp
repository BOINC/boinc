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
#include "filesys.h"

#include "client_state.h"
#include "client_msgs.h"
#include "file_names.h"

#include "cs_notice.h"

using std::vector;
using std::string;
using std::set;

NOTICES notices;
RSS_FEEDS rss_feeds;
RSS_FEED_OP rss_feed_op;

// write a list of feeds to a file
//
static void write_rss_feed_descs(MIOFILE& fout, vector<RSS_FEED>& feeds) {
    if (!feeds.size()) return;
    fout.printf("<rss_feeds>\n");
    for (unsigned int i=0; i<feeds.size(); i++) {
        feeds[i].write(fout);
    }
    fout.printf("</rss_feeds>\n");
}

static void project_feed_list_file_name(PROJECT* p, char* buf) {
    char url[256];
    escape_project_url(p->master_url, url);
    sprintf(buf, "feeds/feeds_%s.xml", url);
}

// write notices newer than seqno as XML (for GUI RPC)
//
void NOTICES::write(int seqno, MIOFILE& fout, bool public_only) {
    unsigned int i;
    fout.printf("<notices>\n");
    for (i=0; i<notices.size(); i++) {
        NOTICE& n = notices[i];
        if (n.seqno <= seqno) break;
        if (public_only && n.is_private) continue;
        n.write(fout, true);
    }
    fout.printf("</notices>\n");
}

void NOTICES::append(NOTICE& n) {
    if (notices.empty()) {
        n.seqno = 1;
    } else {
        n.seqno = notices.front().seqno + 1;
    }
    notices.push_front(n);
}

bool NOTICES::append_unique(NOTICE& n) {
    for (unsigned int i=0; i<notices.size(); i++) {
        NOTICE& n2 = notices[i];
        if (!strcmp(n.guid, n2.guid)) return false;
    }
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO,
            "[notice_debug] appending notice: %s",
            n.title
        );
    }
    append(n);
    return true;
}

static bool cmp(NOTICE n1, NOTICE n2) {
    if (n1.arrival_time < n2.arrival_time) return true;
    if (n1.arrival_time > n2.arrival_time) return false;
    return (strcmp(n1.guid, n2.guid) > 0);
}

void NOTICES::init() {
    // todo: read archive of client and server notices
    rss_feeds.init();

    // sort by decreasing arrival time, then assign seqnos
    //
    sort(notices.begin(), notices.end(), cmp);
    unsigned int n = notices.size();
    for (unsigned int i=0; i<n; i++) {
        notices[i].seqno = n - i;
    }
}

void NOTICES::write_archive(char* url) {
    char buf[256], path[256];

    escape_project_url(url, buf);
    sprintf(path, "feeds/archive_%s", buf);
    FILE* f = fopen(path, "w");
    MIOFILE fout;
    fout.init_file(f);
    fout.printf("<notices>\n");
    if (!f) return;
    for (unsigned int i=0; i<notices.size(); i++) {
        NOTICE& n = notices[i];
        n.write(fout, false);
    }
    fout.printf("</notices>\n");
    fclose(f);
}

// called on startup.  Get list of feeds.  Read archives.
//
void RSS_FEEDS::init() {
    unsigned int i;
    MIOFILE fin;
    FILE* f;

    boinc_mkdir(FEEDS_DIR);
    f = fopen("feeds/feeds.xml", "r");
    if (f) {
        fin.init_file(f);
        parse_rss_feed_descs(fin, feeds);
        fclose(f);
    }

    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        char path[256];
        project_feed_list_file_name(p, path);
        f = fopen(path, "r");
        if (f) {
            fin.init_file(f);
            parse_rss_feed_descs(fin, p->proj_feeds);
            fclose(f);
        }
    }

    // normally the main list is union of the project lists.
    // if it's not, the following will fix
    //
    update();

    for (i=0; i<feeds.size(); i++) {
        RSS_FEED& rf = feeds[i];
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice_debug] feed: %s, %.0f sec",
                rf.url, rf.poll_interval
            );
        }
        rf.read_archive_file();
    }
}

RSS_FEED* RSS_FEEDS::lookup_url(char* url) {
    for (unsigned int i=0; i<feeds.size(); i++) {
        RSS_FEED& rf = feeds[i];
        if (!strcmp(rf.url, url)) {
            return &rf;
        }
    }
    return NULL;
}

// the set of project feeds has changed.
// update the master list.
//
void RSS_FEEDS::update() {
    unsigned int i, j;
    for (i=0; i<feeds.size(); i++) {
        RSS_FEED& rf = feeds[i];
        rf.found = false;
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        for (j=0; j<p->proj_feeds.size(); j++) {
            RSS_FEED& rf = p->proj_feeds[j];
            RSS_FEED* rfp = lookup_url(rf.url);
            if (rfp) {
                rfp->found = true;
            } else {
                rf.found = true;
                feeds.push_back(rf);
                if (log_flags.notice_debug) {
                    msg_printf(0, MSG_INFO,
                        "[notice_debug] adding feed: %s, %.0f sec",
                        rf.url, rf.poll_interval
                    );
                }
            }
        }
    }
    vector<RSS_FEED>::iterator iter = feeds.begin();
    while (iter != feeds.end()) {
        RSS_FEED& rf = *iter;
        if (rf.found) {
            iter++;
        } else {
            // TODO: check if fetch in progress!
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO,
                    "[notice_debug] removing feed: %s",
                    rf.url
                );
            }
            iter = feeds.erase(iter);
        }
    }
    write_feed_list();
}

void RSS_FEEDS::write_feed_list() {
    FILE* f = fopen("feeds/feeds.xml", "w");
    if (!f) return;
    MIOFILE fout;
    fout.init_file(f);
    write_rss_feed_descs(fout, feeds);
    fclose(f);
}

void RSS_FEED::feed_file_name(char* path) {
    char buf[256];
    escape_project_url(url, buf);
    sprintf(path, "feeds/%s", buf);
}

void RSS_FEED::archive_file_name(char* path) {
    char buf[256];
    escape_project_url(url, buf);
    sprintf(path, "feeds/archive_%s", buf);
}

// read and parse the contents of the archive file;
// insert items in NOTICES
//
int RSS_FEED::read_archive_file() {
    char path[256];
    char tag[256];
    bool is_tag;

    archive_file_name(path);
    FILE* f = fopen(path, "r");
    if (!f) {
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice_debug] no archive file for %s", url
            );
        }
        return 0;
    }
    MIOFILE fin;
    fin.init_file(f);
    XML_PARSER xp(&fin);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/notices")) {
            fclose(f);
            return 0;
        }
        if (!strcmp(tag, "notice")) {
            NOTICE n;
            int retval = n.parse(xp);
            if (retval) {
                if (log_flags.notice_debug) {
                    msg_printf(0, MSG_INFO,
                        "[notice_debug] archive item parse error: %d", retval
                    );
                }
            } else {
                notices.append(n);
            }
        }
    }
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "[notice_debug] archive parse error");
    }
    fclose(f);
    return ERR_XML_PARSE;
}

// parse a feed descriptor (in scheduler reply or feed list file)
//
int RSS_FEED::parse_desc(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    strcpy(url, "");
    poll_interval = 0;
    next_poll_time = 0;
    append_seqno = false;
    strcpy(last_seqno, "");
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/rss_feed")) {
            if (!poll_interval || !strlen(url)) {
                if (log_flags.notice_debug) {
                    msg_printf(0, MSG_INFO,
                        "[notice_debug] URL or poll interval missing in sched reply feed"
                    );
                }
                return ERR_XML_PARSE;
            }
            return 0;
        }
        if (xp.parse_str(tag, "url", url, sizeof(url))) continue;
        if (xp.parse_double(tag, "poll_interval", poll_interval)) continue;
        if (xp.parse_double(tag, "next_poll_time", next_poll_time)) continue;
        if (xp.parse_bool(tag, "append_seqno", append_seqno)) continue;
        if (xp.parse_str(tag, "last_seqno", last_seqno, sizeof(last_seqno))) continue;
    }
    return ERR_XML_PARSE;
}

void RSS_FEED::write(MIOFILE& fout) {
    fout.printf(
        "  <rss_feed>\n"
        "    <url>%s</url>\n"
        "    <poll_interval>%f</poll_interval>\n"
        "    <next_poll_time>%f</next_poll_time>\n"
        "    <append_seqno>%d</append_seqno>\n"
        "    <last_seqno>%s</last_seqno>\n"
        "  </rss_feed>\n",
        url,
        poll_interval,
        next_poll_time,
        append_seqno?1:0,
        last_seqno
    );
}

// parse the actual RSS feed.
// Return true if got any new ones.
//
int RSS_FEED::parse_items(XML_PARSER& xp, int& nitems) {
    char tag[256];
    bool is_tag;
    nitems = 0;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/rss")) {
            return 0;
        }
        if (!strcmp(tag, "item")) {
            NOTICE n;
            int retval = n.parse_rss(xp);
            if (!retval) {
                n.arrival_time = gstate.now;
                if (append_seqno) {
                    notices.append(n);
                    nitems++;
                } else {
                    if (notices.append_unique(n)) {
                        nitems++;
                    }
                }
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

RSS_FEED_OP::RSS_FEED_OP() {
    error_num = BOINC_SUCCESS;
    gui_http = &gstate.gui_http;
}

// see if time to start new fetch
//
bool RSS_FEED_OP::poll() {
    unsigned int i;
    if (gstate.gui_http.is_busy()) return false;
    for (i=0; i<rss_feeds.feeds.size(); i++) {
        RSS_FEED& rf = rss_feeds.feeds[i];
        if (gstate.now > rf.next_poll_time) {
            rf.next_poll_time = gstate.now + rf.poll_interval;
            char filename[256];
            rf.feed_file_name(filename);
            rfp = &rf;
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO,
                    "[notice_debug] start fetch from %s", rf.url
                );
            }
            gstate.gui_http.do_rpc(this, rf.url, filename);
        }
    }
    return false;
}

// handle a completed RSS feed fetch
//
void RSS_FEED_OP::handle_reply(int http_op_retval) {
    char filename[256];
    int nitems;

    if (http_op_retval) {
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice_debug] fetch of %s failed: %d", rfp->url, http_op_retval
            );
        }
        return;
    }

    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO,
            "[notice_debug] handling reply from %s", rfp->url
        );
    }

    rfp->feed_file_name(filename);
    FILE* f = fopen(filename, "r");
    MIOFILE fin;
    fin.init_file(f);
    XML_PARSER xp(&fin);
    int retval = rfp->parse_items(xp, nitems);
    if (retval) {
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice_debug] RSS parse error: %d", retval
            );
        }
    }
    fclose(f);

    if (nitems) {
        notices.write_archive(rfp->url);
    }
}

// parse feed descs from scheduler reply or feed list file
//
int parse_rss_feed_descs(MIOFILE& fin, vector<RSS_FEED>& feeds) {
    char tag[256];
    bool is_tag;
    XML_PARSER xp(&fin);
    int retval;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/rss_feeds")) return 0;
        if (!strcmp(tag, "rss_feed")) {
            RSS_FEED rf;
            retval = rf.parse_desc(xp);
            if (retval) {
                if (log_flags.sched_op_debug) {
                    msg_printf(0, MSG_INFO,
                        "[sched_op_debug] error in <rss_feed> element"
                     );
                }
            } else {
                feeds.push_back(rf);
            }
        }
    }
    return ERR_XML_PARSE;
}

static void write_project_feed_list(PROJECT* p) {
    char buf[256];
    project_feed_list_file_name(p, buf);
    FILE* f = fopen(buf, "w");
    if (!f) return;
    MIOFILE fout;
    fout.init_file(f);
    write_rss_feed_descs(fout, p->proj_feeds);
    fclose(f);
}

// A scheduler RPC returned a list (possibly empty) of feeds.
// Add new ones to the project's set,
// and remove ones from the project's set that aren't in the list.
//
void handle_sr_feeds(vector<RSS_FEED>& feeds, PROJECT* p) {
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
                rf2 = rf;
                present = true;
                break;
            }
        }
        if (!present) {
            rf.found = true;
            p->proj_feeds.push_back(rf);
            feed_set_changed = true;
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
            iter = p->proj_feeds.erase(iter);
            feed_set_changed = true;
        }
    }

    // if anything was added or removed, update master set
    //
    if (feed_set_changed) {
        write_project_feed_list(p);
        rss_feeds.update();
    }
}

#ifdef _WIN32
// compensate for lameness
static int month_index(char* x) {
    if (strstr(x, "Jan")) return 0;
    if (strstr(x, "Feb")) return 1;
    if (strstr(x, "Mar")) return 2;
    if (strstr(x, "Apr")) return 3;
    if (strstr(x, "May")) return 4;
    if (strstr(x, "Jun")) return 5;
    if (strstr(x, "Jul")) return 6;
    if (strstr(x, "Aug")) return 7;
    if (strstr(x, "Sep")) return 8;
    if (strstr(x, "Oct")) return 9;
    if (strstr(x, "Nov")) return 10;
    if (strstr(x, "Dev")) return 11;
    return 0;
}

static int parse_rss_time(char* buf) {
    char day_name[64], month_name[64];
    int day_num, year, h, m, s;
    int n = sscanf(buf, "%s %d %s %d %d:%d:%d",
        day_name, &day_num, month_name, &year, &h, &m, &s
    );
    printf("n: %d\n", n);

    struct tm tm;
    tm.tm_sec = s;
    tm.tm_min = m;
    tm.tm_hour = h;
    tm.tm_mday = day_num;
    tm.tm_mon = month_index(month_name);
    tm.tm_year = year-1900;
    tm.tm_wday = 0;
    tm.tm_yday = 0;
    tm.tm_isdst = 0;

    int t = mktime(&tm);
    t -= gstate.host_info.timezone;
    return t;
}
#endif

int NOTICE::parse_rss(XML_PARSER& xp) {
    char tag[1024], buf[256];
    bool is_tag;

    memset(this, 0, sizeof(*this));
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/item")) return 0;
        if (xp.parse_str(tag, "title", title, sizeof(title))) continue;
        if (xp.parse_str(tag, "link", url, sizeof(url))) continue;
        if (xp.parse_str(tag, "guid", guid, sizeof(guid))) continue;
        if (xp.parse_string(tag, "description", description)) continue;
        if (xp.parse_str(tag, "pubDate", buf, sizeof(buf))) {
#ifdef _WIN32
            create_time = parse_rss_time(buf);
#else
            struct tm tm;
            strptime(buf, "%a, %d %b %Y %H:%M:%S", &tm);
            create_time = mktime(&tm);
#endif
            continue;
        }
    }
    return ERR_XML_PARSE;
}
