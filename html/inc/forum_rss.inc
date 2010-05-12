<?php
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

require_once("../inc/forum_db.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");

function show_forum_rss_item($thread, $userid, $threads_only, $truncate) {
    $unique_url=URL_BASE."forum_thread.php?id=".$thread->id;

    $clause2 = " and hidden=0 ";
    if ($userid) $clause2 .= "and user=$userid";
    if ($threads_only) {
        $posts = BoincPost::enum("thread=$thread->id $clause2 order by id limit 1");
    } else {
        $posts = BoincPost::enum("thread=$thread->id $clause2 order by timestamp desc limit 1");
    }
    $post = $posts[0];
    $post_date = gmdate('D, d M Y H:i:s',$post->timestamp).' GMT';
    $t = bb2html($post->content, true);
    if ($truncate) {
        if (strlen($post->content) > 256) {
            $t = substr($post->content, 0, 256).". . .";
        }
    }
    //$t = htmlspecialchars($t);
    echo "<item>
        <title><![CDATA[".strip_tags(bb2html($thread->title))."]]</title>
        <link>$unique_url</link>
        <guid isPermaLink=\"true\">$unique_url</guid>
        <description><![CDATA[\n$t\n]]></description>
        <pubDate>$post_date</pubDate>
        </item>
    ";
}

function forum_rss($forumid, $userid, $truncate, $threads_only, $ndays) {
    $clause = "forum=$forumid ";

    if ($userid) {
        $user = BoincUser::lookup_id($userid);
        if (!$user) error_page("no such user");
        $clause .= " and owner=$userid";
    }

    $tlimit = time() - $ndays*86400;

    if ($threads_only) {
        $q = "$clause and status=0 and hidden=0 and sticky=0 and create_time > $tlimit order by create_time desc";
    } else {
        $q = "$clause and status=0 and hidden=0 and sticky=0 and timestamp > $tlimit order by timestamp desc";
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

    $forum=BoincForum::lookup_id($forumid);
    // Create channel header and open XML content
    //
    $description = PROJECT.": $forum->title";
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
        show_forum_rss_item($thread, $userid, $threads_only, $truncate);
    }

    echo "
        </channel>
        </rss>
    ";
}

?>
