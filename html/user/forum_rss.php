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
require_once("../inc/db.inc");

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
        Include only the <input name=nitems> most recent posts (default: 20).
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
$nitems = get_int('nitems', true);
$truncate = get_str('truncate', true);
$threads_only = get_str('threads_only', true);

if(!$nitems || $nitems < "1" || $nitems > "20") {
    $nitems = "20";
}

$clause = "forum=$forumid ";

if ($userid) {
    $user = BoincUser::lookup_id($userid);
    if (!$user) error_page("no such user");
    $clause .= " and owner=$userid";
}

$db = BoincDb::get();
if ($threads_only) {
    $q = "$clause and status=0 and hidden=0 and sticky=0 order by create_time desc limit $nitems";
} else {
    $q = "$clause and status=0 and hidden=0 and sticky=0 order by timestamp desc limit $nitems";
}

$threads = BoincThread::enum($q);

// Get unix time that last modification was made to the news source
//

// Now construct header
//
header ("Expires: " . gmdate('D, d M Y H:i:s', time()+86400) . " GMT");
if (sizeof($threads)) {
    $t = $threads[0];
    $last_mod_time = $threads_only?$t->create_time:$t->timestamp;
    $create_date  = gmdate('D, d M Y H:i:s', $last_mod_time) . ' GMT'; 
    header ("Last-Modified: " . $create_date);
}
header ("Content-Type: application/xml");

// Create channel header and open XML content
//
$description = PROJECT.": $forum->description";
if ($user) {
    $description .= " (posts by $user->name)";
}
$channel_image = URL_BASE . "rss_image.gif";
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
    <title>".$description."</title>
    <link>".URL_BASE."</link>
    <description>".$description."</description>
    <copyright>".COPYRIGHT_HOLDER."</copyright>
    <lastBuildDate>".$create_date."</lastBuildDate>
    <language>".$language."</language>
    <image>
        <url>".$channel_image."</url>
        <title>".PROJECT."</title>
        <link>".URL_BASE."</link>
    </image>
";

// write news items
//
foreach ($threads as $thread) {
    $unique_url=URL_BASE."forum_thread.php?id=".$thread->id;

    $clause2 = " and hidden=0 ";
    if ($userid) $clause2 .= "and user=$userid";
    if ($threads_only) {
        $posts = BoincPost::enum("thread=$thread->id $clause2 order by id limit 1");
    } else {
        $posts = BoincPost::enum("thread=$thread->id $clause2 order by timestamp desc limit 1");
    }
    $post = $posts[0];
    $post_date=gmdate('D, d M Y H:i:s',$post->timestamp).' GMT';
    if ($truncate) {
        if (strlen($post->content) > 256) {
            $t = substr($post->content, 0, 256).". . .";
        } else {
            $t = $post->content;
        }
        $t = htmlspecialchars(htmlspecialchars(htmlspecialchars($t)));
        // in this case we don't want to convert BBcode to HTML
        // because there might be unclosed tags
    } else {
        $t = htmlspecialchars(bb2html($post->content, true));
    }
    echo "<item>
    <title>".strip_tags($thread->title)."</title>
    <link>$unique_url</link>
    <guid isPermaLink=\"true\">$unique_url</guid>
    <description>$t</description>
    <pubDate>$post_date</pubDate>
</item>
";
}

// Close XML content
//
echo "
    </channel>
    </rss>
";

?>
