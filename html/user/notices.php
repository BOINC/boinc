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

// Generate RSS feed of notices for this user

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
        <link>".secure_url_base()."</link>
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

$since_time = time() - 30*86400;

$user = BoincUser::lookup_id($userid);
if (!$user) xml_error();

// the auth in the URL includes "userid_"
//

$x = $user->id."_".notify_rss_auth($user);
if ($x != $auth) {
    xml_error(-155, 'Invalid authenticator');
}

$since_clause = "and create_time > $since_time";

$notifies = BoincNotify::enum("userid = $userid $since_clause");

// there may be a better way to do this

$items = array();
foreach ($notifies as $n) {
    $i = new StdClass;
    $i->type = 0;
    $i->time = $n->create_time;
    $i->val = $n;
    $items[] = $i;
}

$forum = news_forum();
if ($forum) {
    $threads = BoincThread::enum(
        "forum = $forum->id and hidden=0 and status=0 $since_clause"
    );
    foreach ($threads as $t) {
        $i = new StdClass;
        $i->type = 1;
        $i->time = $t->create_time;
        $i->val = $t;
        $items[] = $i;
    }
}

usort($items, 'notice_cmp');

$client_version = boinc_client_version();
$no_images = $client_version && ($client_version < 70300);

notices_rss_start();
foreach ($items as $item) {
    switch ($item->type) {
    case 0:
        show_notify_rss_item($item->val);
        break;
    case 1:
        show_forum_rss_item($item->val, 0, 1, $no_images);
        break;
    }
}
notices_rss_end();

?>
