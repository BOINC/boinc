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

// Create message boards.
// RUN THIS AS A SCRIPT, NOT VIA A BROWSER.
// TODO: rewrite this using the DB abstraction layer
// First, edit the set of forums (below) and remove the following line

die("edit script to use your forum names, and remove the die()\n");

$cli_only = true;
require_once("../inc/forum_db.inc");
require_once("../inc/util_ops.inc");

function create_category($orderID, $name, $is_helpdesk) {
    $q = "insert into category (orderID, lang, name, is_helpdesk) values ($orderID, 1, '$name', $is_helpdesk)";
    $result = mysql_query($q);
    if (!$result) {
        $cat = BoincCategory::lookup("name='$name' and is_helpdesk=$is_helpdesk");
        if ($cat) return $cat->id;
        echo "can't create category\n";
        echo mysql_error();
        exit();
    }
    return mysql_insert_id();
}

function create_forum($category, $orderID, $title, $description, $is_dev_blog=0) {
    $q = "insert into forum (category, orderID, title, description, is_dev_blog) values ($category, $orderID, '$title', '$description', $is_dev_blog)";
    $result = mysql_query($q);
    if (!$result) {
        $forum = BoincForum::lookup("category=$category and title='$title'");
        if ($forum) return $forum->id;
        echo "can't create forum\n";
        echo mysql_error();
        exit();
    }
    return mysql_insert_id();
}

db_init();

$catid = create_category(0, "", 0);
create_forum($catid, 0, "News", "News from this project", 1);
create_forum($catid, 1, "Science", "Discussion of this project\'s science");
create_forum($catid, 2, "Number crunching", "Credit, leaderboards, CPU performance");
create_forum($catid, 3, "Cafe", "Meet and greet other participants");

$catid = create_category(0, "Platform-specific problems", 1);
create_forum($catid, 0, "Windows", "Installing and running BOINC on Windows");
create_forum($catid, 1, "Unix/Linux", "Installing and running BOINC on Unix and Linux");
create_forum($catid, 2, "Macintosh", "Installing and running BOINC on Mac OS/X");
$catid = create_category(1, "General issues", 1);
create_forum($catid, 3, "Getting started", "Creating your account");
create_forum($catid, 4, "Preferences", "Using preferences");
create_forum($catid, 6, "Web site", "Issues involving this web site");

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
