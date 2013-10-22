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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

check_get_args(array("teamid", "action", "tnow", "ttok"));

function show_admin_page($user, $team) {
    page_head(tra("Team administration for %1", $team->name));
    echo "
        <ul>
        <li><a href=team_edit_form.php?teamid=$team->id>".tra("Edit team info")."</a>
            <br><span class=note>".tra("Change team name, URL, description, type, or country")."</span>
        <li>
            ".tra("Member list:")."
        <a href=team_email_list.php?teamid=$team->id>".tra("HTML")."</a>
        &middot; <a href=team_email_list.php?teamid=$team->id&plain=1>".tra("text")."</a>
            <br><span class=note>".tra("View member names and email addresses")."</span>
        <li>".tra("View change history:")."
            <a href=team_delta.php?teamid=$team->id>".tra("HTML")."</a>
            &middot; <a href=team_delta.php?teamid=$team->id&xml=1>".tra("XML")."</a>
            <br><span class=note>".tra("See when members joined or quit this team")."</span>
    ";

    // founder-only stuff follows
    //
    if ($team->userid == $user->id) {
        $tokens = url_tokens($user->authenticator);
        if ($team->ping_user > 0) {
            $user2 = BoincUser::lookup_id($team->ping_user);
            $deadline = date_str(transfer_ok_time($team));
            echo "<li>
                <a href=team_change_founder_form.php?teamid=$team->id><font color=red><strong>".tra("Respond to foundership request.")."</strong></font></a>  ".tra("If you don't respond by %1, %2 may assume foundership of this team.", $deadline, $user2->name)
                ;
        }
        echo "
            <li><a href=team_remove_inactive_form.php?teamid=$team->id>".tra("Remove members")."</a>
                <br><span class=note>".tra("Remove inactive or unwanted members from this team")."</span>
            <li><a href=team_change_founder_form.php?teamid=$team->id>".tra("Change founder")."</a>
                <br><span class=note>".tra("Transfer foundership to another member")."</span>
            <li><a href=team_admins.php?teamid=$team->id>".tra("Add/remove Team Admins")."</a>
                <br><span class=note>".tra("Give selected team members Team Admin privileges")."</span>

            <li><a href=team_manage.php?teamid=$team->id&action=delete&$tokens>".tra("Remove team")."</a>
                <br><span class=note>".tra("Allowed only if team has no members")."</a>
            <li><a href=team_forum.php?teamid=$team->id&cmd=manage>".tra("Message board")."</a>
                <br><span class=note>".tra("Create or manage a team message board")."</span>
        ";
    }
    echo "

        <p>
        <li>
            ".tra("To have this team created on all BOINC projects (current and future) you can make it into a %1BOINC-wide team%2.", "<a href=http://boinc.berkeley.edu/teams/>", "</a>")."
        <li>
            ".tra("Team admins are encouraged to join and participate in the Google %1boinc-team-founders%2 group.", "<a href=http://groups.google.com/group/boinc-team-founders>", "</a>")."
        <li>
            ".tra("Other resources for BOINC team admins are available from a third-party site, %1www.boincteams.com%2.", "<a href=http://www.boincteams.com>", "</a>")."
    </ul>
    ";

    page_tail();
}

$user = get_logged_in_user(true);
$teamid = get_int('teamid');
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page(tra("No such team"));

$action = get_str('action', true);
if ($action == 'delete') {
    require_founder_login($user, $team);
    if (team_count_members($team->id) > 0) {
        error_page(tra("Can't delete non-empty team"));
    }
    check_tokens($user->authenticator);
    $team->delete();
    page_head(tra("Team %1 deleted", $team->name));
    page_tail();
} else {
    require_admin($user, $team);
    show_admin_page($user, $team);
}
?>
