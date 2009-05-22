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

// get a forum (possibly filtered by user) as RSS feed

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
        <input type=submit value=OK>
    ";
    page_tail();
    exit;
}

$userid = get_int('userid', true);
$nitems = get_int('nitems', true);

if(!$nitems || $nitems < "1" || $nitems > "20") {
    $nitems = "20";
}


$clause = "forum=$forumid ";

if ($userid) {
    $user = BoincUser::lookup_id($userid);
    if (!$user) error_page("no such user");
    $clause .= " and owner=$userid";
}

class Int {
};

$db = BoincDb::get();
$x = $db->lookup_fields(
    "thread",
    "Int",
    "max(timestamp) as foo",
    "$clause and status=0 and hidden=0 and sticky=0"
);
$last_mod_time = $x->foo;

$threads = BoincThread::enum("$clause and status=0 and hidden=0 and sticky=0 order by create_time desc limit $nitems"
);

// Get unix time that last modification was made to the news source
//
$create_date  = gmdate('D, d M Y H:i:s', $last_mod_time) . ' GMT'; 

// Now construct header
//
header ("Expires: " . gmdate('D, d M Y H:i:s', time()+86400) . " GMT");
header ("Last-Modified: " . $create_date);
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
    $post_date=gmdate('D, d M Y H:i:s',$thread->create_time).' GMT';
    $unique_url=URL_BASE."forum_thread.php?id=".$thread->id;

    $clause2 = $userid?"and user=$userid":"";
    $posts = BoincPost::enum("thread=$thread->id $clause2 order by timestamp limit 1");
    $post = $posts[0];
    echo "<item>
    <title>".strip_tags($thread->title)."</title>
    <link>$unique_url</link>
    <guid isPermaLink=\"true\">$unique_url</guid>
    <description>".htmlspecialchars(htmlspecialchars(substr($post->content,0,255)))." . . .</description>
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
