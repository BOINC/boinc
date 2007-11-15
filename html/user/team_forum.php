<?php

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
    $id = BoincForum::insert("(category, parent_type) values ($team->id, 1)");
    $forum = BoincForum::lookup_id($id);
    if (!$forum) {
        error_page("couldn't create forum");
    }
    edit_form($user, $forum);
}

function edit_form($user, $team, $forum) {
    page_head("Team forum");
    echo "
        <form action=team_forum.php method=post>
        <input type=hidden name=teamid value=$team->id>
        <input type=hidden name=cmd value=edit_action>
    ";
    echo form_tokens($user->authenticator);
    start_table();
    //row2("Title", "<input name=title value=\"$forum->title\">");
    //row2("Description", "<textarea name=description>$forum->description</textarea>");
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
    page_tail();
}

function edit_action($forum) {
    //$title = post_str('title');
    //$title = BoincDb::escape_string($title);
    //$description = post_str('description');
    //$description = BoincDb::escape_string($description);
    $post_min_interval = post_int('post_min_interval');
    $post_min_total_credit = post_int('post_min_total_credit');
    $post_min_expavg_credit = post_int('post_min_expavg_credit');
    $ret = $forum->update("post_min_interval=$post_min_interval, post_min_total_credit=$post_min_total_credit, post_min_expavg_credit=$post_min_expavg_credit");
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
        edit_form($user, $team, $forum);
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
} else if ($cmd != "") {
    error_page("unknown command $cmd");
} else {
    show_forum($team);
}

?>
