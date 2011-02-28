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

require_once("../project/project.inc");

function news_item($date, $title, $post) {
    echo "<p>";
    if ($title) {
        echo "<span class=news_title>$title</span><br>\n";
    }
    $d = time_str($date);
    if (is_string($post)) {
        $forum_link = false;
        $text = output_transform($post);
    } else {
        $forum_link = true;
        $text = output_transform($post->content);
    }
    echo "
        <span class=news_content>$text</span>
        <span class=news_date>$d
    ";
    if ($forum_link) {
        echo "
            &middot; <a href=".URL_BASE."forum_thread.php?id=$post->thread> ".tra("Comment")."</a>
        ";
    }
    echo "</span>
        <br clear=all>
        <hr size=0 class=news_line>
    ";
}

function news_forum() {
    if (defined("NEWS_FORUM_NAME")) {
        $forum_name = NEWS_FORUM_NAME;
    } else {
        $forum_name = "News";
    }
    return BoincForum::lookup("parent_type=0 and title = '$forum_name'");
}

function is_news_forum($forum) {
    if (defined("NEWS_FORUM_NAME")) {
        $forum_name = NEWS_FORUM_NAME;
    } else {
        $forum_name = "News";
    }
    return $forum->parent_type == 0 && ($forum->title === $forum_name);
}

function show_news($start, $count) {
    $forum = news_forum();
    if (!$forum) {
        echo "
            No news forum.  Run html/ops/create_forums.php.
        ";
        return;
    }
    
    if (is_array($start)) {
        echo "You have not changed show_news() to use 0 instead of
            deprecated \$project_news as the first param.";
        return;
    }
    
    $lim = "";
    if ($start) {
        if ($count) {
            $lim = "limit $start, $count";
        } else {
            $lim = "limit $start, 99999999";
        }
    } else if ($count) {
        $lim = "limit $count";
    }
    $threads = BoincThread::enum("forum = $forum->id and hidden=0 order by id desc $lim");

    foreach ($threads as $thread) {
        $posts = BoincPost::enum("thread=$thread->id order by id limit 1");
        $post = $posts[0];
        if (strstr($post->content, $thread->title) == $post->content) {
            $title = null;
        } else {
            $title = $thread->title;
        }
        news_item($thread->create_time, $title, $post);
    }

    if ($count) {
        echo "<a href=".URL_BASE."old_news.php>... more</a>";
    }
    echo "
        <br><br><p class=\"smalltext\">
    ";
    printf(tra("News is available as an %sRSS feed%s"),
        "<a href=rss_main.php>",
        " &nbsp; <img src=\"img/rss_icon.gif\" alt=\"RSS\"></a>"
    );
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
