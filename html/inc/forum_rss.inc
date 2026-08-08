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

require_once("../inc/forum.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/text_transform.inc");

// return true if the given HTML may contain images or video
//
function contains_image_or_video($x) {
    if (strstr($x, "<img ")) return true;
    if (strstr($x, "<object ")) return true;
    if (strstr($x, "<iframe ")) return true;
    return false;
}

function show_forum_rss_item($thread, $userid, $threads_only, $no_images) {
    $unique_url=secure_url_base()."forum_thread.php?id=".$thread->id;

    $clause2 = " and hidden=0 ";
    if ($userid) $clause2 .= "and user=$userid";
    if ($threads_only) {
        $posts = BoincPost::enum("thread=$thread->id $clause2 order by id limit 1");
    } else {
        $posts = BoincPost::enum("thread=$thread->id $clause2 order by timestamp desc limit 1");
    }
    if (!count($posts)) return;
    $post = $posts[0];
    $post_date = gmdate('D, d M Y H:i:s',$post->timestamp).' GMT';
    $post_user = BOincUser::lookup_id($post->user);
    BoincForumPrefs::lookup($post_user);
    $options = new output_options();
    if (is_admin($post_user)) {
        $options->htmlitems = false;
    }
    $t = output_transform($post->content, $options);
    if ($no_images && contains_image_or_video($t)) return;
    echo "<item>
        <title><![CDATA[".sanitize_tags(bb2html($thread->title))."]]></title>
        <link>$unique_url</link>
        <guid isPermaLink=\"true\">$unique_url</guid>
        <description><![CDATA[\n$t\n]]></description>
        <pubDate>$post_date</pubDate>
        </item>
    ";
}

function forum_rss($forumid, $userid, $threads_only, $ndays) {
    $clause = "forum=$forumid ";

    if ($userid) {
        $user = BoincUser::lookup_id($userid);
        if (!$user) error_page("no such user");
        $clause .= " and owner=$userid";
    }

    if ($ndays) {
        $tlimit = time() - $ndays*86400;
    } else {
        $tlimit = 0;
    }

    if ($threads_only) {
        $q = "$clause and hidden=0 and create_time > $tlimit order by create_time desc";
    } else {
        $q = "$clause and hidden=0 and timestamp > $tlimit order by timestamp desc";
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
    } else {
        $create_date  = gmdate('D, d M Y H:i:s') . ' GMT';
    }
    header ("Content-Type: application/xml");

    $forum=BoincForum::lookup_id($forumid);
    // Create channel header and open XML content
    //
    $description = PROJECT.": $forum->title";
    if ($userid) {
        $description .= " (posts by $user->name)";
    }
    $channel_image = secure_url_base() . "rss_image.gif";
    $language = "en-us";
    echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>
        <rss version=\"2.0\">
        <channel>
        <title>".$description."</title>
        <description>".$description."</description>
        <link>".secure_url_base()."</link>
        <copyright>".COPYRIGHT_HOLDER."</copyright>
        <lastBuildDate>".$create_date."</lastBuildDate>
        <language>".$language."</language>
        <image>
            <url>".$channel_image."</url>
            <title>".PROJECT."</title>
            <link>".secure_url_base()."</link>
        </image>
    ";

    foreach ($threads as $thread) {
        show_forum_rss_item($thread, $userid, $threads_only, false);
    }

    echo "
        </channel>
        </rss>
    ";
}

?>
