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
    if (n1.arrival_time > n2.arrival_time) return true;
    if (n1.arrival_time < n2.arrival_time) return false;
    return (strcmp(n1.guid, n2.guid) > 0);
}

static void project_feed_list_file_name(PROJ_AM* p, char* buf) {
    char url[256];
    escape_project_url(p->master_url, url);
    sprintf(buf, "notices/feeds_%s.xml", url);
}

// parse feed descs from scheduler reply or feed list file
//
int parse_rss_feed_descs(XML_PARSER& xp, vector<RSS_FEED>& feeds) {
    int retval;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/rss_feeds")) return 0;
        if (xp.match_tag("rss_feed")) {
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

static void write_project_feed_list(PROJ_AM* p) {
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
void handle_sr_feeds(vector<RSS_FEED>& feeds, PROJ_AM* p) {
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
                rf2.found = true;
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
    if (strstr(x, "Dec")) return 11;
    return 0;
}
#endif

// convert a date-time string (assumed GMT) to Unix time

static int parse_rss_time(char* buf) {
#ifdef _WIN32
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

    return (int)mktime(&tm);
#else
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(buf, "%a, %d %b %Y %H:%M:%S", &tm);
    return mktime(&tm);
#endif
}

///////////// NOTICE ////////////////

int NOTICE::parse_rss(XML_PARSER& xp) {
    char buf[256];

    clear();
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/item")) return 0;
        if (xp.parse_str("title", title, sizeof(title))) continue;
        if (xp.parse_str("link", link, sizeof(link))) continue;
        if (xp.parse_str("guid", guid, sizeof(guid))) continue;
        if (xp.parse_string("description", description)) continue;
        if (xp.parse_str("pubDate", buf, sizeof(buf))) {
            create_time = parse_rss_time(buf);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

///////////// NOTICES ////////////////

// called at the start of client initialization
//
void NOTICES::init() {
#if 0
    read_archive_file(NOTICES_DIR"/archive.xml", NULL);
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "read %d BOINC notices", (int)notices.size());
    }
    write_archive(NULL);
#endif
}

// called at the end of client initialization
//
void NOTICES::init_rss() {
    rss_feeds.init();
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "read %d total notices", (int)notices.size());
    }

    // sort by decreasing arrival time, then assign seqnos
    //
    sort(notices.begin(), notices.end(), cmp);
    size_t n = notices.size();
    for (unsigned int i=0; i<n; i++) {
        notices[i].seqno = (int)(n - i);
    }
}

// return true if strings are the same after discarding digits.
// This eliminates showing
// "you need 25 GB more disk space" and
// "you need 24 GB more disk space" as separate notices.
//
static inline bool string_equal_nodigits(string& s1, string& s2) {
    const char *p = s1.c_str();
    const char *q = s2.c_str();
    while (1) {
        if (isascii(*p) && isdigit(*p)) {
            p++;
            continue;
        }
        if (isascii(*q) && isdigit(*q)) {
            q++;
            continue;
        }
        if (!*p || !*q) break;
        if (*p != *q) return false;
        p++;
        q++;
    }
    if (*p || *q) return false;
    return true;
}

static inline bool same_text(NOTICE& n1, NOTICE& n2) {
    if (strcmp(n1.title, n2.title)) return false;
    if (!string_equal_nodigits(n1.description, n2.description)) return false;
    return true;
}

void NOTICES::clear_keep() {
    deque<NOTICE>::iterator i = notices.begin();
    while (i != notices.end()) {
        NOTICE& n = *i;
        n.keep = false;
        i++;
    }
}

void NOTICES::unkeep(const char* url) {
    deque<NOTICE>::iterator i = notices.begin();
    bool removed_something = false;
    while (i != notices.end()) {
        NOTICE& n = *i;
        if (!strcmp(url, n.feed_url) && !n.keep) {
            i = notices.erase(i);
            removed_something = true;
        } else {
            i++;
        }
    }
#ifndef SIM
    if (removed_something) {
        gstate.gui_rpcs.set_notice_refresh();
    }
#endif
}

static inline bool same_guid(NOTICE& n1, NOTICE& n2) {
    if (!strlen(n1.guid)) return false;
    return !strcmp(n1.guid, n2.guid);
}

// we're considering adding a notice n.
// If there's already an identical message n2
//     return false (don't add n)
// If there's a message n2 with same title and text,
//      and n is significantly newer than n2,
//      delete n2
//
// Also remove notices older than 30 days
//
bool NOTICES::remove_dups(NOTICE& n) {
    deque<NOTICE>::iterator i = notices.begin();
    bool removed_something = false;
    bool retval = true;
    double min_time = gstate.now - 30*86400;
    while (i != notices.end()) {
        NOTICE& n2 = *i;
        if (n2.arrival_time < min_time
            || (n2.create_time && n2.create_time < min_time)
        ) {
            i = notices.erase(i);
            removed_something = true;
        } else if (same_guid(n, n2)) {
            n2.keep = true;
            return false;
        } else if (same_text(n, n2)) {
            int min_diff = 0;

            // show a given scheduler notice at most once a week
            //
            if (!strcmp(n.category, "scheduler")) {
                min_diff = 7*86400;
            }

            if (n.create_time > n2.create_time + min_diff) {
                i = notices.erase(i);
                removed_something = true;
            } else {
                n2.keep = true;
                retval = false;
                ++i;
            }
        } else {
            ++i;
        }
    }
#ifndef SIM
    if (removed_something) {
        gstate.gui_rpcs.set_notice_refresh();
    }
#endif
    return retval;
}

// add a notice.
// 
bool NOTICES::append(NOTICE& n) {
    if (!remove_dups(n)) {
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
            n.seqno, strlen(n.title)?n.title:n.description.c_str()
        );
    }
    notices.push_front(n);
#if 0
    if (!strlen(n.feed_url)) {
        write_archive(NULL);
    }
#endif
    return true;
}


// read and parse the contents of an archive file.
// If rfp is NULL it's a system msg, else a feed msg.
// insert items in NOTICES
//
int NOTICES::read_archive_file(const char* path, RSS_FEED* rfp) {
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
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/notices")) {
            fclose(f);
            return 0;
        }
        if (xp.match_tag("notice")) {
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
                append(n);
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

// Remove "need network access" notices
//
void NOTICES::remove_network_msg() {
    deque<NOTICE>::iterator i = notices.begin();
    while (i != notices.end()) {
        NOTICE& n = *i;
        if (!strcmp(n.description.c_str(), NEED_NETWORK_MSG)) {
            i = notices.erase(i);
#ifndef SIM
            gstate.gui_rpcs.set_notice_refresh();
#endif
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO, "REMOVING NETWORK MESSAGE");
            }
        } else {
            ++i;
        }
    }
}

// write notices newer than seqno as XML (for GUI RPC).
// Write them in order of increasing seqno
//
void NOTICES::write(int seqno, GUI_RPC_CONN& grc, bool public_only) {
    size_t i;
    MIOFILE mf;

    if (!net_status.need_physical_connection) {
        remove_network_msg();
    }
    if (log_flags.notice_debug) {
        msg_printf(0, MSG_INFO, "NOTICES::write: seqno %d, refresh %s, %d notices",
            seqno, grc.get_notice_refresh()?"true":"false", (int)notices.size()
        );
    }
    grc.mfout.printf("<notices>\n");
    if (grc.get_notice_refresh()) {
        grc.clear_notice_refresh();
        NOTICE n;
        n.seqno = -1;
        seqno = -1;
        i = notices.size();
        n.write(grc.mfout, true);
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO, "NOTICES::write: sending -1 seqno notice");
        }
    } else {
        for (i=0; i<notices.size(); i++) {
            NOTICE& n = notices[i];
            if (n.seqno <= seqno) break;
        }
    }
    for (; i>0; i--) {
        NOTICE& n = notices[i-1];
        if (public_only && n.is_private) continue;
        if (log_flags.notice_debug) {
            msg_printf(0, MSG_INFO, "NOTICES::write: sending notice %d", n.seqno);
        }
        n.write(grc.mfout, true);
    }
    grc.mfout.printf("</notices>\n");
}

///////////// RSS_FEED ////////////////

void RSS_FEED::feed_file_name(char* path) {
    char buf[256];
    escape_project_url(url_base, buf);
    sprintf(path, NOTICES_DIR"/%s.xml", buf);
}

void RSS_FEED::archive_file_name(char* path) {
    char buf[256];
    escape_project_url(url_base, buf);
    sprintf(path, NOTICES_DIR"/archive_%s.xml", buf);
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
    strcpy(url, "");
    poll_interval = 0;
    next_poll_time = 0;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/rss_feed")) {
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
        if (xp.parse_str("url", url, sizeof(url))) {
            xml_unescape(url);
        }
        if (xp.parse_double("poll_interval", poll_interval)) continue;
        if (xp.parse_double("next_poll_time", next_poll_time)) continue;
    }
    return ERR_XML_PARSE;
}

void RSS_FEED::write(MIOFILE& fout) {
    char buf[256];
    strcpy(buf, url);
    xml_escape(url, buf, sizeof(buf));
    fout.printf(
        "  <rss_feed>\n"
        "    <url>%s</url>\n"
        "    <poll_interval>%f</poll_interval>\n"
        "    <next_poll_time>%f</next_poll_time>\n"
        "  </rss_feed>\n",
        buf,
        poll_interval,
        next_poll_time
    );
}

static inline bool create_time_asc(NOTICE n1, NOTICE n2) {
    return n1.create_time < n2.create_time;
}

// parse the actual RSS feed.
//
int RSS_FEED::parse_items(XML_PARSER& xp, int& nitems) {
    nitems = 0;
    int ntotal = 0, nerror = 0;
    int retval, func_ret = ERR_XML_PARSE;
    vector<NOTICE> new_notices;

    notices.clear_keep();

    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/rss")) {
            if (log_flags.notice_debug) {
                msg_printf(0, MSG_INFO,
                    "[notice] parsed RSS feed: total %d error %d added %d",
                    ntotal, nerror, nitems
                );
            }
            func_ret = 0;
            break;
        }
        if (xp.match_tag("item")) {
            NOTICE n;
            ntotal++;
            retval = n.parse_rss(xp);
            if (retval) {
                nerror++;
            } else if (n.create_time < gstate.now - 30*86400) {
                if (log_flags.notice_debug) {
                    msg_printf(0, MSG_INFO,
                        "[notice] item is older than 30 days: %s",
                        n.title
                    );
                }
            } else {
                n.arrival_time = gstate.now;
                n.keep = true;
                strcpy(n.feed_url, url);
                strcpy(n.project_name, project_name);
                new_notices.push_back(n);
            }
            continue;
        }
        if (xp.parse_int("error_num", retval)) {
            if (log_flags.notice_debug) {
                msg_printf(0,MSG_INFO,
                    "[notice] RSS fetch returned error %d (%s)",
                    retval,
                    boincerror(retval)
                );
            }
            return retval;
        }
    }

    //  sort new notices by increasing create time, and append them
    //
    std::sort(new_notices.begin(), new_notices.end(), create_time_asc);
    for (unsigned int i=0; i<new_notices.size(); i++) {
        NOTICE& n = new_notices[i];
        if (notices.append(n)) {
            nitems++;
        }
    }
    notices.unkeep(url);
    return func_ret;
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
            strcpy(url, rf.url);
            gstate.gui_http.do_rpc(this, url, filename, true);
            break;
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
    if (!f) {
        msg_printf(0, MSG_INTERNAL_ERROR,
            "RSS feed file '%s' not found", filename
        );
        return;
    }
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

    notices.write_archive(rfp);
}

///////////// RSS_FEEDS ////////////////

static void init_proj_am(PROJ_AM* p) {
    FILE* f;
    MIOFILE fin;
    char path[256];

    project_feed_list_file_name(p, path);
    f = fopen(path, "r");
    if (f) {
        fin.init_file(f);
        XML_PARSER xp(&fin);
        parse_rss_feed_descs(xp, p->proj_feeds);
        fclose(f);
    }
}

// called on startup.  Get list of feeds.  Read archives.
//
void RSS_FEEDS::init() {
    unsigned int i;

    boinc_mkdir(NOTICES_DIR);

    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        init_proj_am(p);
    }
    if (gstate.acct_mgr_info.using_am()) {
        init_proj_am(&gstate.acct_mgr_info);
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

// arrange to fetch the project's feeds
//
void RSS_FEEDS::trigger_fetch(PROJ_AM* p) {
    for (unsigned int i=0; i<p->proj_feeds.size(); i++) {
        RSS_FEED& rf = p->proj_feeds[i];
        RSS_FEED* rfp = lookup_url(rf.url);
        if (rfp) {
            rfp->next_poll_time = 0;
        }
    }
}

void RSS_FEEDS::update_proj_am(PROJ_AM* p) {
    unsigned int j;
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

// the set of project feeds has changed.
// update the master list.
//
void RSS_FEEDS::update_feed_list() {
    unsigned int i;
    for (i=0; i<feeds.size(); i++) {
        RSS_FEED& rf = feeds[i];
        rf.found = false;
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        update_proj_am(p);
    }
    if (gstate.acct_mgr_info.using_am()) {
        update_proj_am(&gstate.acct_mgr_info);
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
                if (rss_feed_op.gui_http->is_busy()) {
                    gstate.http_ops->remove(&rss_feed_op.gui_http->http_op);
                }
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
