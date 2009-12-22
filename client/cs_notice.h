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
// 3) the client (MSG_USER_ERROR messages)
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
// feeds/feeds.xml              feed list
// feeds/feeds_PROJ_URL.xml     list of project feeds
// feeds/archive_RSS_URL.xml    item archive for a feed
// feeds/out_RSS_URL.xml        result of last fetch for a feed

#include <deque>
#include <vector>

#include "miofile.h"

#include "gui_http.h"

#include "notice.h"

struct PROJECT;

struct NOTICES {
    std::deque<NOTICE> notices;
    void write(int seqno, MIOFILE&, bool public_only);
    void append(NOTICE&);
    void append_unique(NOTICE&);
    void init();
};

extern NOTICES notices;

struct RSS_FEED {
    char url[256];
    double poll_interval;
    double next_poll_time;
    bool append_seqno;
        // if true, append "?seqno=x" to feed requests;
        // assume we'll get only unique items
    bool duplicate;
        // some other project has this same feed,
        // and this one is a duplicate.
    bool found;
        // temp used in garbage collection

    int fetch_start();
    int fetch_complete();

    void write(MIOFILE&);
    int parse_desc(XML_PARSER&);
    int parse_items(XML_PARSER&);
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

struct RSS_FEEDS {
    std::vector<RSS_FEED> feeds;
    void init();
    void update();
    RSS_FEED* lookup_url(char*);
};

extern RSS_FEEDS rss_feeds;

int parse_notice_feeds(MIOFILE& fin, std::vector<RSS_FEED>&);
void handle_sr_feeds(std::vector<RSS_FEED>&, PROJECT*);
    // process the feeds in a scheduler reply

#endif
