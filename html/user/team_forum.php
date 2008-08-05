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

// create, manage, or read a team message board

require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/forum_db.inc");

function create_confirm($user, $team) {
    page_head("Create Message Board");
    echo "
        You may create a Message Board for use by $team->name.
        <ul>
        <li> Only team members will be able to post.
        <li> At your option, only members will be able to read.
        <li> You and your Team Admins will have moderator privileges.
        </ul>
    ";
    $tokens = url_tokens($user->authenticator);
    show_button(
        "team_forum.php?teamid=$team->id&cmd=create$tokens",
        "Create Message Board",
        "Create a Message Board for $team->name"
    );
    page_tail();
}

function create_forum($user, $team) {
    $f = BoincForum::lookup("parent_type=1 and category=$team->id");
    if ($f) {
        error_page("Team already has a forum");
    }
    $id = BoincForum::insert("(category, parent_type) values ($team->id, 1)");
    $forum = BoincForum::lookup_id($id);
    if (!$forum) {
        error_page("couldn't create forum");
    }
    edit_form($user, $team, $forum, true);
}

function edit_form($user, $team, $forum, $first) {
    page_head("Team forum");
    echo "
        <form action=team_forum.php method=post>
        <input type=hidden name=teamid value=$team->id>
        <input type=hidden name=cmd value=edit_action>
    ";
    echo form_tokens($user->authenticator);
    start_table();
    if (!strlen($forum->title)) $forum->title = $team->name;
    if (!strlen($forum->description)) $forum->description = "Discussion among members of $team->name";
    row2("Title", "<input name=title value=\"$forum->title\">");
    row2("Description", "<textarea name=description>$forum->description</textarea>");
    row2("Minimum time between posts (seconds)",
        "<input name=post_min_interval value=$forum->post_min_interval>"
    );
    row2("Minimum total credit to post",
        "<input name=post_min_total_credit value=$forum->post_min_total_credit>"
    );
    row2("Minimum average credit to post",
        "<input name=post_min_expavg_credit value=$forum->post_min_expavg_credit>"
    );
    row2("", "<input type=submit value=OK>");
    end_table();
    echo "
        </form>
    ";
    if (!$first) {
        echo "
            <p>
            <a href=team_forum.php?teamid=$team->id&cmd=remove_confirm$tokens>
            Remove your team's message board.</a>
        ";
    }
    page_tail();
}

function remove_confirm($user, $team) {
    $tokens = url_tokens($user->authenticator);
    page_head("Really remove message board?");
    echo "
        Are you sure you want to remove your team's message board?
        All threads and posts will be permanently removed.
        (You may, however, create a new message board later).
        <p>
        <a href=team_forum.php?teamid=$team->id&cmd=remove>Yes - remove message board</a>
    ";
    page_tail();
}

function remove($team) {
    $forum = BoincForum::lookup("parent_type=1 and category=$team->id");
    if (!$forum) error_page("not found");
    
    // delete threads and posts
    //
    $threads = BoincThread::enum("forum=$forum->id");
    foreach ($threads as $thread) {
        $posts = BoincPost::enum("thread=$thread->id");
        foreach ($posts as $post) {
            $post->delete();
        }
        $thread->delete();
    }
    $forum->delete();

    page_head("Message board removed");
    page_tail();
}

function edit_action($forum) {
    $title = strip_tags(post_str('title'));
    $title = BoincDb::escape_string($title);
    $description = strip_tags(post_str('description'));
    $description = BoincDb::escape_string($description);
    $post_min_interval = post_int('post_min_interval');
    $post_min_total_credit = post_int('post_min_total_credit');
    $post_min_expavg_credit = post_int('post_min_expavg_credit');
    $ret = $forum->update("title='$title', description='$description', post_min_interval=$post_min_interval, post_min_total_credit=$post_min_total_credit, post_min_expavg_credit=$post_min_expavg_credit");
    if ($ret) {
        page_head("Team Message Board Updated");
        echo "Update successful";
        page_tail();
    } else {
        error_page("update failed");
    }
}

function show_forum($team) {
    $forum = BoincForum::lookup("parent_type=1 and category=$team->id");
    if (!$forum) {
        error_page("team has no forum");
    }
    Header("Location: forum_forum.php?id=$forum->id");
}

$teamid = get_int("teamid", true);
if (!$teamid) $teamid = post_int('teamid');

$team = BoincTeam::lookup_id($teamid);
if (!$team) {
    error_page("no such team");
}

$cmd = get_str('cmd', true);
if (!$cmd) $cmd = post_str('cmd', true);

if ($cmd == 'manage') {
    $user = get_logged_in_user();
    require_founder_login($user, $team);
    $forum = BoincForum::lookup("parent_type=1 and category=$teamid");
    if (!$forum) {
        create_confirm($user, $team);
    } else {
        edit_form($user, $team, $forum, false);
    }
} else if ($cmd == 'create') {
    $user = get_logged_in_user();
    check_tokens($user->authenticator);
    require_founder_login($user, $team);
    create_forum($user, $team);
} else if ($cmd == 'edit_action') {
    $user = get_logged_in_user();
    require_founder_login($user, $team);
    check_tokens($user->authenticator);
    $forum = BoincForum::lookup("parent_type=1 and category=$teamid");
    if (!$forum) error_page("no forum");
    edit_action($forum);
} else if ($cmd == "remove_confirm") {
    $user = get_logged_in_user();
    require_founder_login($user, $team);
    remove_confirm($user, $team);
} else if ($cmd == "remove") {
    $user = get_logged_in_user();
    require_founder_login($user, $team);
    remove($team);
} else if ($cmd != "") {
    error_page("unknown command $cmd");
} else {
    show_forum($team);
}

?>
