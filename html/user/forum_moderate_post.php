<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// The form where a moderator decides what he is going to do to a post.
// Submits information to forum_moderate_post_action.php for actual action
// to be done.

require_once('../inc/util.inc');
require_once('../inc/forum.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array("id", "action", "userid", "tnow", "ttok"));

$logged_in_user = get_logged_in_user();
check_tokens($logged_in_user->authenticator);
BoincForumPrefs::lookup($logged_in_user);
$postid = get_int('id');
$post = BoincPost::lookup_id($postid);
if (!$post) error_page('No such post');
$thread = BoincThread::lookup_id($post->thread);
$forum = BoincForum::lookup_id($thread->forum);

if (!get_str('action')) {
    error_page("No action specified");
}
if (!is_moderator($logged_in_user, $forum)) {
    error_page("You are not authorized to moderate this post.");
}

page_head(tra("Moderate post"));

echo "<form action=\"forum_moderate_post_action.php?id=".$post->id."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->authenticator);
start_table();

$get_reason = true;
if (get_str('action')=="hide") {
    //display input that selects reason
    echo "<input type=hidden name=action value=hide>";
    row1(tra("Hide post"));
    row2(tra("Reason"),
        select_from_array('category',
            array(
                "",
                tra("Obscene"),
                tra("Flame/Hate mail"),
                tra("Commercial spam"),
                tra("Doublepost"),
                tra("User Request"),
                tra("Other"),
            )
        )
    );
} elseif (get_str('action')=="move") {
    row1(tra("Move post"));
    echo "<input type=hidden name=action value=move>";
    row2(tra("Destination thread ID:"), "<input name=\"threadid\">");
    // TODO: display where to move the post as a dropdown instead of having to get ID
} elseif (get_str('action')=="banish_user") {
    $userid = get_int('userid');
    $user = BoincUser::lookup_id($userid);
    if (!$user) {
        error_page("no user found");
    }
    BoincForumPrefs::lookup($user);
    $x = $user->prefs->banished_until;
    if ($x>time()) {
        error_page(tra("User is already banished"));
    }
    row1(tra("Banish user"));
    row1(tra("Are you sure you want to banish %1 ?<br/>This will prevent %1 from posting for chosen time period.<br/>It should be done only if %1 has consistently exhibited trollish behavior.", $user->name));
    row2(tra("Ban duration"), "<select class=\"form-control\" name=\"duration\">
            <option value=\"21600\">".tra("6 hours")."</option>
            <option value=\"43200\">".tra("12 hours")."</option>
            <option value=\"86400\">".tra("1 day")."</option>
            <option value=\"604800\">".tra("1 week")."</option>
            <option value=\"1209600\" selected=\"selected\">".tra("2 weeks")."</option>
            <option value=\"2592000\">".tra("1 month")."</option>
            <option value=\"-1\">".tra("Forever")."</option>
        </select>");
    echo "<input type=\"hidden\" name=\"action\" value=\"banish_user\">\n";
    echo "<input type=\"hidden\" name=\"id\" value=\"".$postid."\">\n";
    echo "<input type=\"hidden\" name=\"userid\" value=\"".$userid."\">\n";
    echo "<input type=\"hidden\" name=\"confirmed\" value=\"yes\">\n";
} elseif (get_str('action')=="delete") {
    echo "<input type=hidden name=action value=delete>";
    row2(
        "Are you sure want to delete this post?  This cannot be undone.",
        "<input class=\"btn btn-danger\" type=\"submit\" name=\"submit\" value=\"".tra("OK")."\">"
    );
    $get_reason = false;
} else {
    error_page("Unknown action");
}

if ($get_reason) {
    row2(tra("Optional explanation %1 This is included in email to user.%2", "<br><small>", "</small>"),
        '<textarea name="reason" class="form-control" rows="10"></textarea>'
    );
    row2(
        "",
        "<input class=\"btn btn-success\" type=\"submit\" name=\"submit\" value=\"".tra("OK")."\">"
    );
}

end_table();

echo "</form>";

page_tail();

?>
