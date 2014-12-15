<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// get a forum as RSS feed
// arguments:
// threads_only
//      If true, only show threads (not posts within a thread)
//          by decreasing create time
//      Else enumerate threads by decreasing timestamp,
//          and show the post with latest timestamp for each

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/forum_rss.inc");

if (DISABLE_FORUMS) error_page("Forums are disabled");

$forumid = get_int('forumid');
$forum = BoincForum::lookup_id($forumid);
if (!$forum) error_page("no such forum");

if (get_int('setup', true)) {
    page_head(tra("%1 RSS feed", $forum->title));
    echo tra("This message board is available as an RSS feed.")
        ." "
        .tra("Options:")."
        <form action=\"forum_rss.php\" method=\"get\">
        <input type=\"hidden\" name=\"forumid\" value=\"$forumid\">
        <p>
        ".tra("Include only posts by user ID %1 (default: all users).", "<input name=\"userid\">")."
        <p>
        ".tra("Include only posts from the last %1 days (default: 30).", "<input name=\"ndays\">")."
        <p>
        ".tra("Threads only: %1 (Include only the first post of every thread)", "<input type=\"checkbox\" name=\"threads_only\">")."
        <p>
        <input class=\"btn btn-default\" type=\"submit\" value=\"".tra("OK")."\">
    ";
    page_tail();
    exit;
}

$userid = get_int('userid', true);
$ndays = get_int('ndays', true);
$threads_only = get_str('threads_only', true);

if(!$ndays || $ndays < 1) {
    $ndays = 30;
}

forum_rss($forumid, $userid, $threads_only, $ndays);

?>
