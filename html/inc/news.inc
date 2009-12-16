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

function news_item($date, $title, $text) {
    if ($title) {
        echo "<span class=news_title>$title</span>\n";
    }
    $d = time_str($date);
    $text = bb2html($text);
    echo "
        <br><span class=news_date>$d</span>
        <br><span class=news_content>$text</span>
        <br><br>
    ";
}

function show_news($start, $count) {
    if (defined("NEWS_FORUM_NAME")) {
        $forum_name = NEWS_FORUM_NAME;
    } else {
        $forum_name = "News";
    }
    $forum = BoincForum::lookup("parent_type=0 and title = '$forum_name'");
    if (!$forum) {
        echo "
            No news forum.  Run html/ops/create_forums.php.
        ";
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
    $threads = BoincThread::enum("forum = $forum->id order by id desc $lim");

    foreach ($threads as $thread) {
        $posts = BoincPost::enum("thread=$thread->id order by id limit 1");
        $post = $posts[0];
        if (strstr($post->content, $thread->title)) {
            $title = null;
        } else {
            $title = $thread->title;
        }
        news_item($thread->create_time, $title, $post->content);
    }

    if ($count) {
        echo "<a href=\"old_news.php\">...more</a>";
    }
    echo "
        <p class=\"smalltext\">
        News is available as an
        <a href=\"rss_main.php\">RSS feed</a> <img src=\"img/rss_icon.gif\" alt=\"RSS\">.</p>
    ";
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
