<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
// truncate
//      If true, truncate posts to 256 chars and show BBcode
//      else show whole post and convert to HTML

require_once("../project/project.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/forum_rss.inc");

$forumid = get_int('forumid');
$forum = BoincForum::lookup_id($forumid);
if (!$forum) error_page("no such forum");

if (get_int('setup', true)) {
    page_head("$forum->name RSS feed");
    echo "
        This message board is available as an RSS feed.
        Options:
        <form action=forum_rss.php>
        <input type=hidden name=forumid value=$forumid>
        <p>
        Include only posts by user ID <input name=userid> (default: all users).
        <p>
        Include only posts from the last <input name=ndays> days (default: 30).
        <p>
        Truncate posts <input type=checkbox name=truncate checked>
        <p>
        Threads only <input type=checkbox name=threads_only>
        <p>
        <input type=submit value=OK>
    ";
    page_tail();
    exit;
}

$userid = get_int('userid', true);
$ndays = get_int('ndays', true);
$truncate = get_str('truncate', true);
$threads_only = get_str('threads_only', true);

if(!$ndays || $ndays < "1") {
    $ndays = "30";
}

forum_rss($forumid, $userid, $truncate, $threads_only, $ndays);

?>
