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

require_once('../inc/util.inc');
require_once('../inc/forum_db.inc');
require_once('../inc/forum_banishment_vote.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array("action", "userid"));

$config = get_config();

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

if (!get_str('action')) {
    error_page("You must specify an action...");
}
if (!$logged_in_user->prefs->privilege(S_MODERATOR)) {
    // Can't moderate without being moderator
    error_page(tra("You are not authorized to banish users."));
}

$userid = get_int('userid');
$user = BoincUser::lookup_id($userid);
if (!$user) error_page('No such user');

page_head(tra("Banishment Vote"));

echo "<form action=\"forum_banishment_vote_action.php?userid=".$userid."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->authenticator);
start_table();
row1(tra("Banishment Vote"));

if (get_str('action')=="start") {
    if (!$user) {
        error_page(tra("No user with this ID found."));
    }
    BoincForumPrefs::lookup($user);
    $x = $user->prefs->banished_until;
    if ($x>time()) {
        error_page(tra("User is already banished"));
    }
    //display input that selects reason
    echo "<input type=hidden name=action value=start>";
    echo "<input type=\"hidden\" name=\"userid\" value=\"".$userid."\">\n";
    row1(tra("Are you sure you want to banish %1 ?<br/>This will prevent %1 from posting for chosen time period.<br/>It should be done only if %1 has consistently exhibited trollish behavior.", $user->name));
    row2("",
    tra("Select the reason category, optionally write a longer description of why the user should be banished."));
    row2(tra("Category"),
    "<select class=\"form-control\" name=\"category\">
    <option value=\"1\">".tra("Obscene")."</option>
    <option value=\"2\">".tra("Flame/Hate mail")."</option>
    <option value=\"3\">".tra("User Request")."</option>
    <option value=\"4\">".tra("Other")."</option>
</select>");
row2(tra("Reason")."<br>".tra("Mailed if nonempty"),
    '<textarea name="reason" class="form-control" rows="10"></textarea>'
);

row2(
    "",
    "<input class=\"btn btn-default\" type=\"submit\" name=\"submit\" value=\"".tra("Proceed with vote")."\">"
);
} elseif (get_str('action')=="yes") {
    vote_yes($config,$logged_in_user,$user);
} elseif (get_str('action')=="no") {
    vote_no($config,$logged_in_user,$user);
} else {
    error_page("Unknown action");
}

end_table();

echo "</form>";

page_tail();

?>
