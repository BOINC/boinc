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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <string>
#include <set>
#endif

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
using std::deque;

NOTICES notices;
RSS_FEEDS rss_feeds;
RSS_FEED_OP rss_feed_op;

////////////// UTILITY FUNCTIONS ///////////////

static bool cmp(NOTICE n1, NOTICE n2) {
    if (n1.arrival_time < n2.arrival_time) return true;
    if (n1.arrival_time > n2.arrival_time) return false;
    return (strcmp(n1.guid, n2.guid) > 0);
}

static void project_feed_list_file_name(PROJECT* p, char* buf) {
    char url[256];
    escape_project_url(p->master_url, url);
    sprintf(buf, "notices/feeds_%s.xml", url);
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
                        "[sched_op] error in <rss_feed> element"
                     );
                }
            } else {
                feeds.push_back(rf);
            }
        }
    }
    return ERR_XML_PARSE;
}

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
        rss_feeds.update_feed_list();
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
    sscanf(buf, "%s %d %s %d %d:%d:%d",
        day_name, &day_num, month_name, &year, &h, &m, &s
    );

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

    int t = (int)mktime(&tm);
    t -= gstate.host_info.timezone;
    return t;
}
#endif

///////////// NOTICE ////////////////

int NOTICE::parse_rss(XML_PARSER& xp) {
    char tag[1024], buf[256];
    bool is_tag;

    clear();
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/item")) return 0;
        if (xp.parse_str(tag, "title", title, sizeof(title))) continue;
        if (xp.parse_str(tag, "link", link, sizeof(link))) continue;
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

///////////// NOTICES ////////////////

void NOTICES::init() {
    read_archive_file(NOTICES_DIR"/archive.xml", NULL);
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "read %d BOINC notices", (int)notices.size());
    }
    write_archive(NULL);
}

void NOTICES::init_rss() {
    rss_feeds.init();
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "read %d total notices", (int)notices.size());
    }

    // sort by decreasing arrival time, then assign seqnos
    //
    sort(notices.begin(), notices.end(), cmp);
    unsigned int n = notices.size();
    for (unsigned int i=0; i<n; i++) {
        notices[i].seqno = n - i;
    }
}

static inline bool equivalent(NOTICE& n1, NOTICE& n2) {
    if (strcmp(n1.title, n2.title)) return false;
    if (n1.description != n2.description) return false;
    return true;
}

// we're considering adding a notice n.
// See if an equivalent notice n2 is already there; if so:
// keep_old: return false
// !keep_old: delete n2
//
// Also remove notices older than 30 days
//
bool NOTICES::remove_dups(NOTICE& n, bool keep_old) {
    deque<NOTICE>::iterator i = notices.begin();
    bool removed_something = false;
    bool retval = true;
    while (i != notices.end()) {
        NOTICE& n2 = *i;
        if (n2.arrival_time < gstate.now - 30*86400) {
            i = notices.erase(i);
            removed_something = true;
        } else if (equivalent(n, n2)) {
            if (keep_old) {
                retval = false;
                ++i;
            } else {
                i = notices.erase(i);
                removed_something = true;
            }
        } else {
            ++i;
        }
    }
    if (removed_something) {
        gstate.gui_rpcs.set_notice_refresh();
    }
    return retval;
}

// add a notice.
// If an identical notice is already there:
// - if keep_old is set, use the existing one unless it's really old
//   otherwise delete the old one and add the new one
//
// This is called from various places:
//  client_msgs.cpp:
// 
bool NOTICES::append(NOTICE& n, bool keep_old) {
    if (!remove_dups(n, keep_old)) {
        return false;
    }
    if (notices.empty()) {
        n.seqno = 1;
    } else {
        n.seqno = notices.front().seqno + 1;
    }
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO,
            "[notice] appending notice %d: %s",
            n.seqno, n.title
        );
    }
    notices.push_front(n);
    if (!strlen(n.feed_url)) {
        write_archive(NULL);
    }
    return true;
}


// read and parse the contents of an archive file.
// If rfp is NULL it's a system msg, else a feed msg.
// insert items in NOTICES
//
int NOTICES::read_archive_file(const char* path, RSS_FEED* rfp) {
    char tag[256];
    bool is_tag;

    FILE* f = fopen(path, "r");
    if (!f) {
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice] no archive file %s", path
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
                        "[notice] archive item parse error: %d", retval
                    );
                }
            } else {
                if (rfp) {
                    strcpy(n.feed_url, rfp->url);
                    strcpy(n.project_name, rfp->project_name);
                }
                append(n, false);
            }
        }
    }
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "[notice] archive parse error");
    }
    fclose(f);
    return ERR_XML_PARSE;
}

// write archive file for the given RSS feed
// (or, if NULL, non-RSS notices)
//
void NOTICES::write_archive(RSS_FEED* rfp) {
    char path[256];

    if (rfp) {
        rfp->archive_file_name(path);
    } else {
        strcpy(path, NOTICES_DIR"/archive.xml");
    }
    FILE* f = fopen(path, "w");
    if (!f) return;
    MIOFILE fout;
    fout.init_file(f);
    fout.printf("<notices>\n");
    if (!f) return;
    for (unsigned int i=0; i<notices.size(); i++) {
        NOTICE& n = notices[i];
        if (rfp) {
            if (strcmp(rfp->url, n.feed_url)) continue;
        } else {
            if (strlen(n.feed_url)) continue;
        }
        n.write(fout, false);
    }
    fout.printf("</notices>\n");
    fclose(f);
}

// write notices newer than seqno as XML (for GUI RPC).
// Write them in order of increasing seqno
//
void NOTICES::write(int seqno, MIOFILE& fout, bool public_only, bool notice_refresh) {
    unsigned int i;
    fout.printf("<notices>\n");
    if (notice_refresh) {
        NOTICE n;
        n.seqno = -1;
        seqno = -1;
        i = notices.size();
    } else {
        for (i=0; i<notices.size(); i++) {
            NOTICE& n = notices[i];
            if (n.seqno <= seqno) break;
        }
    }
    for (; i>0; i--) {
        NOTICE& n = notices[i-1];
        if (public_only && n.is_private) continue;
        n.write(fout, true);
    }
    fout.printf("</notices>\n");
}

///////////// RSS_FEED ////////////////

void RSS_FEED::feed_file_name(char* path) {
    char buf[256];
    escape_project_url(url_base, buf);
    sprintf(path, NOTICES_DIR"/%s", buf);
}

void RSS_FEED::archive_file_name(char* path) {
    char buf[256];
    escape_project_url(url_base, buf);
    sprintf(path, NOTICES_DIR"/archive_%s", buf);
}

// read and parse the contents of the archive file;
// insert items in NOTICES
//
int RSS_FEED::read_archive_file() {
    char path[256];
    archive_file_name(path);
    return notices.read_archive_file(path, this);
}

// parse a feed descriptor (in scheduler reply or feed list file)
//
int RSS_FEED::parse_desc(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    strcpy(url, "");
    poll_interval = 0;
    next_poll_time = 0;
    use_seqno = false;
    strcpy(last_seqno, "");
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/rss_feed")) {
            if (!poll_interval || !strlen(url)) {
                if (log_flags.notice_debug) {
                    msg_printf(0, MSG_INFO,
                        "[notice] URL or poll interval missing in sched reply feed"
                    );
                }
                return ERR_XML_PARSE;
            }
            strcpy(url_base, url);
            char* p = strchr(url_base, '?');
            if (p) *p = 0;
            return 0;
        }
        if (xp.parse_str(tag, "url", url, sizeof(url))) continue;
        if (xp.parse_double(tag, "poll_interval", poll_interval)) continue;
        if (xp.parse_double(tag, "next_poll_time", next_poll_time)) continue;
        if (xp.parse_bool(tag, "use_seqno", use_seqno)) continue;
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
        "    <use_seqno>%d</use_seqno>\n"
        "    <last_seqno>%s</last_seqno>\n"
        "  </rss_feed>\n",
        url,
        poll_interval,
        next_poll_time,
        use_seqno?1:0,
        last_seqno
    );
}

// parse the actual RSS feed.
//
int RSS_FEED::parse_items(XML_PARSER& xp, int& nitems) {
    char tag[256];
    bool is_tag;
    nitems = 0;
    int ntotal = 0, nerror = 0;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/rss")) {
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO,
                    "[notice] parsed RSS feed: total %d error %d added %d",
                    ntotal, nerror, nitems
                );
            }
            return 0;
        }
        if (!strcmp(tag, "item")) {
            NOTICE n;
            ntotal++;
            int retval = n.parse_rss(xp);
            if (retval) {
                nerror++;
            } else {
                n.arrival_time = gstate.now;
                strcpy(n.feed_url, url);
                if (notices.append(n, true)) {
                    nitems++;
                }
            }
            continue;
        }
        if (xp.parse_str(tag, "seqno", last_seqno, sizeof(last_seqno))) {
            continue;
        }
    }
    return ERR_XML_PARSE;
}

///////////// RSS_FEED_OP ////////////////

RSS_FEED_OP::RSS_FEED_OP() {
    error_num = BOINC_SUCCESS;
    gui_http = &gstate.gui_http;
}

// see if time to start new fetch
//
bool RSS_FEED_OP::poll() {
    unsigned int i;
    if (gstate.gui_http.is_busy()) return false;
    if (gstate.network_suspended) return false;
    for (i=0; i<rss_feeds.feeds.size(); i++) {
        RSS_FEED& rf = rss_feeds.feeds[i];
        if (gstate.now > rf.next_poll_time) {
            rf.next_poll_time = gstate.now + rf.poll_interval;
            char filename[256];
            rf.feed_file_name(filename);
            rfp = &rf;
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO,
                    "[notice] start fetch from %s", rf.url
                );
            }
            char url[256];
            if (rf.use_seqno) {
                sprintf(url, "%s?seqno=%s", rf.url, rf.last_seqno);
            } else {
                strcpy(url, rf.url);
            }
            gstate.gui_http.do_rpc(this, url, filename);
        }
    }
    return false;
}

// handle a completed RSS feed fetch
//
void RSS_FEED_OP::handle_reply(int http_op_retval) {
    char filename[256];
    int nitems;

    if (!rfp) return;   // op was canceled

    if (http_op_retval) {
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice] fetch of %s failed: %d", rfp->url, http_op_retval
            );
        }
        return;
    }

    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO,
            "[notice] handling reply from %s", rfp->url
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
                "[notice] RSS parse error: %d", retval
            );
        }
    }
    fclose(f);

    if (nitems) {
        notices.write_archive(rfp);
    }
}

///////////// RSS_FEEDS ////////////////

// called on startup.  Get list of feeds.  Read archives.
//
void RSS_FEEDS::init() {
    unsigned int i;
    MIOFILE fin;
    FILE* f;

    boinc_mkdir(NOTICES_DIR);

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

    update_feed_list();

    for (i=0; i<feeds.size(); i++) {
        RSS_FEED& rf = feeds[i];
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO,
                "[notice] feed: %s, %.0f sec",
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
void RSS_FEEDS::update_feed_list() {
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
                strcpy(rf.project_name, p->get_project_name());
                feeds.push_back(rf);
                if (log_flags.notice_debug) {
                    msg_printf(0, MSG_INFO,
                        "[notice] adding feed: %s, %.0f sec",
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
            // cancel op if active
            //
            if (rss_feed_op.rfp == &(*iter)) {
                rss_feed_op.rfp = NULL;
            }
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO,
                    "[notice] removing feed: %s",
                    rf.url
                );
            }
            iter = feeds.erase(iter);
        }
    }
    write_feed_list();
}

void RSS_FEEDS::write_feed_list() {
    FILE* f = fopen(NOTICES_DIR"/feeds.xml", "w");
    if (!f) return;
    MIOFILE fout;
    fout.init_file(f);
    write_rss_feed_descs(fout, feeds);
    fclose(f);
}
