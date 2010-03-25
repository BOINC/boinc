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

#ifndef _CS_NOTICE_
#define _CS_NOTICE_

// Code related to "notices", which come from
// 1) RSS feeds specified by projects
// 2) Scheduler replies (high-priority messages)
// 3) the client (MSG_USER_ALERT messages)
//
// Classes:
//
// NOTICE represents a notice of any the above types.
// Attributes include a "private" flag,
// and an "arrival time"; for RSS items, this is the time it
// arrived at the client, not the create time.
//
// NOTICES represents the set of all notices in the last 30 days.
// Each notice has a unique seqno, which is a total ordering
// compatible with increasing arrival time.
// GUI RPC allow the enumerating of notices above a given seqno.
// Seqnos are not permanent.
//
// RSS_FEED represents an RSS feed.
// The client polls each feed periodically.
// A feed may have a (nonstandard) <use_seqno> attribute.
// If present, each nonempty feed reply includes an opaque <seqno> element,
// representing the last item returned.
// The following request should include this element in the URL;
// only later elements will be returned.
//
// The last 30 days of each feed is cached on disk.
//
// Two projects may request the same feed.
// So each PROJECT has its own list of feeds.
// There's also a merged list "rss_feeds" where seqno is stored.
//
// files:
// notices/feeds.xml              feed list
// notices/feeds_PROJ_URL.xml     list of project feeds
// notices/archive_RSS_URL.xml    item archive for a feed
// notices/out_RSS_URL.xml        result of last fetch for a feed

#include <deque>
#include <vector>

#include "miofile.h"

#include "gui_http.h"

#include "notice.h"

struct NOTICES {
    std::deque<NOTICE> notices;
        // stored newest (i.e. highest seqno) message first
    void write(int seqno, MIOFILE&, bool public_only, bool notice_refresh);
    bool append(NOTICE&, bool keep_old);
    void init();
    void init_rss();
    int read_archive_file(const char* file, struct RSS_FEED*);
    void write_archive(struct RSS_FEED*);
    bool remove_dups(NOTICE&, bool keep_old);
};

extern NOTICES notices;

struct RSS_FEED {
    char url[256];
    char url_base[256];
    char project_name[256];
    double poll_interval;
    double next_poll_time;
    bool use_seqno;
        // if true, append "?seqno=x" to feed requests;
        // assume we'll get only unique items
    char last_seqno[256];
    bool found;
        // temp used in garbage collection

    int fetch_start();
    int fetch_complete();

    void write(MIOFILE&);
    int parse_desc(XML_PARSER&);
    int parse_items(XML_PARSER&, int&);
    void feed_file_name(char*);
    void archive_file_name(char*);
    int read_archive_file();
};

struct RSS_FEED_OP: public GUI_HTTP_OP {
    int error_num;
    RSS_FEED* rfp;

    RSS_FEED_OP();
    virtual ~RSS_FEED_OP(){}
    virtual void handle_reply(int http_op_retval);
    bool poll();
};

extern RSS_FEED_OP rss_feed_op;

struct RSS_FEEDS {
    std::vector<RSS_FEED> feeds;
    void init();
    void update_feed_list();
    RSS_FEED* lookup_url(char*);
    void write_feed_list();
};

extern RSS_FEEDS rss_feeds;

int parse_rss_feed_descs(MIOFILE& fin, std::vector<RSS_FEED>&);
void handle_sr_feeds(std::vector<RSS_FEED>&, struct PROJECT*);
    // process the feeds in a scheduler reply

#endif
