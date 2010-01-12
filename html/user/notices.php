<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/notify.inc");
require_once("../inc/forum_rss.inc");

function notice_cmp($a, $b) {
    return $a->time < $b->time;
}

function notices_rss_start() {
    $t = gmdate('D, d M Y H:i:s', time())." GMT";
    header("Expires: $t");
    header("Last-Modified: $t");
    header("Content-Type: application/xml");
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
        <rss version=\"2.0\">
        <channel>
        <title>".PROJECT." notices</title>
        <link>".URL_BASE."</link>
        <description>Notices</description>
        <lastBuildDate>$t</lastBuildDate>
    ";
}

function notices_rss_end() {
    echo "
        </channel>
        </rss>
    ";
}

$userid = get_int('userid');
$auth = get_str('auth');
$seqno = get_int('auth', true);

$user = BoincUser::lookup_id($userid);
if (!$user) xml_error();
//if (notify_rss_auth($user) != $auth) xml_error();

$seqno_clause = $seqno?"and create_time > $seqno":"";

$notifies = BoincNotify::enum("userid = $userid $seqno_clause");

$forum = news_forum();
if ($forum) {
    $threads = BoincThread::enum(
        "forum = $forum->id and hidden=0 $seqno_clause"
    );
}

// there may be a better way to do this

$items = array();
foreach ($notifies as $n) {
    $i = null;
    $i->type = 0;
    $i->time = $n->create_time;
    $i->val = $n;
    $items[] = $i;
}

foreach ($threads as $t) {
    $i = null;
    $i->type = 1;
    $i->time = $t->create_time;
    $i->val = $t;
    $items[] = $i;
}

usort($items, 'notice_cmp');

notices_rss_start();
foreach ($items as $item) {
    switch ($item->type) {
    case 0:
        show_notify_rss_item($item->val);
        break;
    case 1:
        show_forum_rss_item($item->val, 0, 1, 0);
        break;
    }
}
notices_rss_end();

?>
