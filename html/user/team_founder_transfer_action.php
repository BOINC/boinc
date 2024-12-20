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

// action = 'initiate_transfer':
//    handle a user's request to initiate a foundership transfer
// action = 'finalize_transfer':
//    handle a user's request to finalize a foundership transfer
// action = 'decline':
//    handle the current founder's declining of the request

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");
require_once("../inc/pm.inc");

if (DISABLE_TEAMS) error_page("Teams are disabled");

check_get_args(array());

$user = get_logged_in_user();
if (!$user->teamid) {
    error_page(tra("You must be a member of a team to access this page."));
}

function send_founder_transfer_email($team, $user, $founder) {

    // send founder a private message for good measure

    $subject = "Team founder transfer request";
    $body = "Team member ".$user->name." has asked that you
transfer foundership of $team->name.
Please go [url=".secure_url_base()."team_change_founder_form.php?teamid=$team->id]here[/url] to grant or decline the request.

If you do not respond within 60 days, ".$user->name." will
be allowed to become the team founder.
";

    pm_send_msg($user, $founder, $subject, $body, false);

    $subject = PROJECT." team founder transfer";
    $body = "Team member ".$user->name." has asked that you
transfer foundership of $team->name in ".PROJECT.".
Please visit
".secure_url_base()."team_change_founder_form.php?teamid=".$team->id."
to grant or decline the request.

If you do not respond within 60 days, ".$user->name." will
be allowed to become the team founder.

Please do not respond to this email.
The mailbox is not monitored and the email
was sent using an automated system.";
    return send_email($founder, $subject, $body);
}

function send_founder_transfer_decline_email($team, $user) {
    $body = "The founder of ".$team->name." has declined your request
to become the founder in ".PROJECT.".
You can repeat the request at least 90 days after the initial request.

Please do not respond to this email.
The mailbox is not monitored and the email
was sent using an automated system.";

    return send_email($user, PROJECT." team founder transfer declined", $body);
}

$action = post_str("action");

switch ($action) {
case "initiate_transfer":
    $team = BoincTeam::lookup_id($user->teamid);
    $founder = BoincUser::lookup_id($team->userid);
    if (!$founder) {
        // no founder - request is granted immediately
        //
        $team->update("userid=$user->id");
        page_head("Team founder request granted");
        echo "You are now the founder of $team->name<p>";
        break;
    }
    $now = time();
    if (new_transfer_request_ok($team, $now)) {
        page_head(tra("Requesting foundership of %1", $team->name));
        $success = send_founder_transfer_email($team, $user, $founder);

        // Go ahead with the transfer even if the email send fails.
        // Otherwise it would be impossible to rescue a team
        // whose founder email is invalid
        //
        $team->update("ping_user=$user->id, ping_time=$now");
        echo "<p>".tra("The current founder has been notified of your request by email and private message.<br /><br />
                       If the founder does not respond within 60 days you will be allowed to become the founder.")
        ."</p>\n";
    } else {
        error_page(tra("Foundership request not allowed now"));
    }
    break;
case "finalize_transfer":
    $team = BoincTeam::lookup_id($user->teamid);
    $now = time();
    if ($user->id == $team->ping_user && transfer_ok($team, $now)) {
        page_head(tra("Assumed foundership of %1", $team->name));
        $team->update("userid=$user->id, ping_user=0, ping_time=0");
        echo tra("Congratulations, you are now the founder of team %1. Go to %2 Your Account page %3 to find the Team Admin options.",
            $team->name,
            sprintf('<a href="%s%s">', secure_url_base(), HOME_PAGE),
            "</a>"
        );
    } else {
        error_page(tra("Foundership request not allowed now"));
    }
    break;
case "decline":
    $teamid = post_int("teamid");
    $team = BoincTeam::lookup_id($teamid);
    require_founder_login($user, $team);
    page_head(tra("Decline founder change request"));

    if ($team->ping_user) {
        $ping_user = BoincUser::lookup_id($team->ping_user);

        $team->update("ping_user=0");
        send_founder_transfer_decline_email($team, $ping_user);
        echo "<p>".tra("The foundership request from %1 has been declined.", user_links($ping_user))
        ."</p>";
    } else {
        echo "<p>".tra("There were no foundership requests.")."</p>";
    }
    break;
default:
    error_page("undefined action ".htmlspecialchars($action));
}

echo "<a href='team_display.php?teamid=$team->id'>".tra("Return to team page")."</a>";

page_tail();


?>
