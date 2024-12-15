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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

if (DISABLE_TEAMS) error_page("Teams are disabled");

check_get_args(array());

$user = get_logged_in_user();
$team = BoincTeam::lookup_id($user->teamid);
if (!$team) {
    error_page(tra("You need to be a member of a team to access this page."));
}

page_head(tra("Request foundership of %1", $team->name));
$now = time();

// it should never happen, but just in case
//
if (!$team->userid) {
    $team->update("userid=$user->id, ping_user=0, ping_time=0");
    echo tra("You are now founder of team %1.", $team->name);
    page_tail();
    exit;
}

if ($user->id == $team->ping_user) {
    echo "<p>".tra("You requested the foundership of %1 on %2.", $team->name, date_str($team->ping_time))."
    </p>";
    if (transfer_ok($team, $now)) {
        echo tra("60 days have elapsed since your request, and the founder has not responded. You may now assume foundership by clicking here:")
            ."<form method=\"post\" action=\"team_founder_transfer_action.php\">
            <input type=\"hidden\" name=\"action\" value=\"finalize_transfer\">
            <input class=\"btn btn-default\" type=\"submit\" value=\"".tra("Assume foundership")."\">
            </form>
        ";
    } else {
        echo "<p>".tra("The founder was notified of your request. If he/she does not respond by %1 you will be given an option to become founder.", date_str(transfer_ok_time($team)))
        ."</p>";
    }
} else {
    if (new_transfer_request_ok($team, $now)) {
        echo "<form method=\"post\" action=\"team_founder_transfer_action.php\">";
        echo "<p>".tra("If the team founder is not active and you want to assume the role of founder, click the button below. The current founder will be sent an email detailing your request, and will be able to transfer foundership to you or to decline your request. If the founder does not respond in 60 days, you will be allowed to become the founder.<br /><br />
                       Are you sure you want to request foundership?")
        ."</p>";

        echo "<input type=\"hidden\" name=\"action\" value=\"initiate_transfer\">
            <input class=\"btn btn-default\" type=\"submit\" value=\"".tra("Request foundership")."\">
            </form>
        ";
    } else {
        if ($team->ping_user) {
            if ($team->ping_user < 0) {
                $team->ping_user = -$team->ping_user;
            }
            $ping_user = BoincUser::lookup_id($team->ping_user);
            echo "<p>".tra("Founder change has already been requested by %1 on %2.", user_links($ping_user), date_str($team->ping_time))
            ."</p>";
        } else {
            echo "<p>".tra("A foundership change was requested during the last 90 days, so new requests are not allowed. Please try again later.")
            ."</p>";
        }
    }
}

echo "<p><a href=\"team_display.php?teamid=".$team->id."\">".tra("Return to team page")."</a>";

page_tail();

?>
