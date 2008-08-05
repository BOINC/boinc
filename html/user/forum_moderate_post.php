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


// The form where a moderator decides what he is going to do to a post.
// Submits informaiton to forum_moderate_post_action.php for actual action
// to be done.

require_once('../inc/forum.inc');

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);
$postid = get_int('id');
$post = BoincPost::lookup_id($postid);
$thread = BoincThread::lookup_id($post->thread);
$forum = BoincForum::lookup_id($thread->forum);

if (!get_str('action')) {
    error_page("No action specified");
}
if (!is_moderator($logged_in_user, $forum)) {
    error_page("You are not authorized to moderate this post.");
}    

page_head('Moderate post');

echo "<form action=\"forum_moderate_post_action.php?id=".$post->id."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->authenticator);
start_table();

if (get_str('action')=="hide") {
    //display input that selects reason
    echo "<input type=hidden name=action value=hide>";
    row1("Hide post");
    row2("Reason",
    "<select name=\"category\">
    <option value=\"1\">Obscene</option>
    <option value=\"2\">Flame/Hate mail</option>
    <option value=\"3\">Commercial spam</option>
    <option value=\"4\">Doublepost</option>
    <option value=\"5\">User Request</option>
    <option value=\"6\">Other</option>
</select>");
} elseif (get_str('action')=="move") {
    row1("Move post");
    echo "<input type=hidden name=action value=move>";
    row2("Destination thread ID:", "<input name=\"threadid\">");
    //todo display where to move the post as a dropdown instead of having to get ID    
} elseif (get_str('action')=="banish_user") {
    $userid = get_int('userid');
    $user = BoincUser::lookup_id($userid);
    BoincForumPrefs::lookup($user);
    if (!$user) {
        error_page("no user");
    }
    $x = $user->prefs->banished_until;
    if ($x>time()) {
        error_page("User is already banished");
    }
    row1("Banish user");
    row1("Are you sure you want to banish ".$user->name."?
        This will prevent ".$user->name." from posting for chosen time period.<br />
        It should be done only if ".$user->name."
        has consistently exhibited trollish behavior.");
    row2("Ban duration", "<select name=\"duration\">
            <option value=\"14400\">4 hours</option>
            <option value=\"86400\">1 day</option>
            <option value=\"604800\">1 week</option>
            <option value=\"1209600\" selected=\"selected\">2 weeks</option>
            <option value=\"2592000\">1 month</option>
            <option value=\"-1\">Forever</option>
        </select>");
    echo "<input type=\"hidden\" name=\"action\" value=\"banish_user\">\n";
    echo "<input type=\"hidden\" name=\"id\" value=\"".$postid."\">\n";
    echo "<input type=\"hidden\" name=\"userid\" value=\"".$userid."\">\n";
    echo "<input type=\"hidden\" name=\"confirmed\" value=\"yes\">\n";
} else {
    error_page( "Unknown action");
}

row2("Optional explanation
    <br><span class=note>This is included in email to user</span>",
    "<textarea name=\"reason\" rows=\"10\" cols=\"80\"></textarea>");

row2(
    "",
    "<input type=\"submit\" name=\"submit\" value=\"OK\">"
);

end_table();

echo "</form>";

page_tail();

?>
