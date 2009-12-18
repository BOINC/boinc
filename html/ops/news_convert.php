#!/usr/bin/env php
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

// This script converts the old file-based news (project_news.inc)
// into the new forum-based format.

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../project/project_news.inc");
require_once("../inc/forum_db.inc");
require_once("../inc/forum.inc");
require_once("../inc/bbcode_convert.inc");

$forum_name = "News";
$forum_desc = "News from this project";

echo "This script exports project news from the project_news.inc file
to a message board.
Do you want to do this (y/n)? ";

$stdin = fopen("php://stdin", "r");
$x = trim(fgets($stdin));
if ($x != "y") {
    exit;
}

while (1) {
    echo "Enter the email address of admin account to appear as poster: ";
    $x = trim(fgets($stdin));
    $user = BoincUser::lookup("email_addr='$x'");
    if (!$user) {
        echo "No such user\n";
        continue;
    }
    BoincForumPrefs::lookup($user);
    if (!$user->prefs->privilege(S_ADMIN)) {
        echo "User doesn't have admin privileges";
        continue;
    }
    break;
}

$category = BoincCategory::lookup("orderID=0 and is_helpdesk=0");
if (!$category) {
    die("can't find category");
}

$forum = BoincForum::lookup("parent_type=0 and title='$forum_name'");
if ($forum) {
    die("News forum already exists");
}

$now = time();
$forum_id = BoincForum::insert("(category, orderID, title, description, timestamp, is_dev_blog, parent_type) values ($category->id, -1, '$forum_name', '$forum_desc', $now, 1, 0)");
$forum = BoincForum::lookup_id($forum_id);

foreach (array_reverse($project_news) as $item) {
    $content = $item[1];
    if (isset($item[2])) {
        $title = $item[2];
    } else {
        $n = strpos($content, ". ");
        if ($n) {
            $title = substr($content, 0, $n);
        } else {
            $title = $content;
        }
    }
    $when = strtotime($item[0]);
    $title = html_to_bbcode($title);
    $title = str_replace("\n", " ", $title);
    $title = mysql_real_escape_string($title);
    $content = html_to_bbcode($content);
    $content = str_replace("\n", " ", $content);
    $content = mysql_real_escape_string($content);

    $thread_id  = BoincThread::insert("(forum, owner, title, create_time, timestamp, replies) values ($forum_id, $user->id, '$title', $when, $when, 0)");
    if (!$thread_id) {
        echo "thread insert failed\n";
        echo "title: [$title]\n";
        echo "when: $when\n";
        exit;
    }

    $id = BoincPost::insert("(thread, user, timestamp, content) values ($thread_id, $user->id, $when, '$content')");
    if (!$id) die("post insert");

    $forum->update("threads=threads+1, posts=posts+1");
}

echo "

Project news has been successfully converted from
html/project/project_news.inc to forum format.
Change your index.php to use
   show_news(0, 5)
to show news and related links.

If everything looks OK, you can delete html/project/project_news.inc

";
?>
