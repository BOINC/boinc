<?php

// action = 'transfer':
//    handle a user's request to initiate a foundership transfer 
// action = 'decline':
//    handle the current founder's declining of the request

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");
require_once("../inc/pm.inc");

$user = get_logged_in_user();
if (!$user->teamid) {
    error_page("You must be a member of a team to access this page.");
}

function send_founder_transfer_email($team, $user) {
    $body = "Team member ".$user->name." has asked that you
transfer foundership of $team->name in ".PROJECT.".
Please visit
".URL_BASE."team_change_founder_form.php?teamid=".$team->id."
to grant or decline the request.
    
If you do not respond within 60 days, ".$user->name." will
be allowed to become the team founder.
    
Please do not respond to this email.
The mailbox is not monitored and the email
was sent using an automated system.";
    
    $subject = PROJECT." team founder transfer";
    $founder = lookup_user_id($team->userid);

    // send founder a private message for good measure

    $body = "Team member ".$user->name." has asked that you
transfer foundership of $team->name.
Please go [url=".URL_BASE."team_change_founder_form.php?teamid=$team->id]here[/url] to grant or decline the request.
    
If you do not respond within 60 days, ".$user->name." will
be allowed to become the team founder.
";

    pm_send($founder, $subject, $body);
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
    $now = time();
    if (new_transfer_request_ok($team, $now)) {
        page_head("Requesting foundership of ".$team->name);
        $success = send_founder_transfer_email($team, $user);

        // Go ahead with the transfer even if the email send fails.
        // Otherwise it would be impossible to rescue a team
        // whose founder email is invalid
        //
        $team->update("ping_user=$user->id, ping_time=$now");
        echo "<p>
            The current founder has been notified of your request by email
            and private message.
            <p>
            If the founder does not respond within 60 days you will be
            allowed to become the founder.
            <p>
        ";
    } else {
        error_page("Foundership request not allowed now");
    }
    break;
case "finalize_transfer":
    $team = BoincTeam::lookup_id($user->teamid);
    $now = time();
    if ($user->id == $team->ping_user && transfer_ok($team, $now)) {
        page_head("Assumed foundership of ".$team->name);
        $team->update("userid=$user->id, ping_user=0, ping_time=0");
        echo "
            Congratulations, you are now the founder of team ".$team->name."
            Go to <a href=\"".URL_BASE."home.php\">Your Account page</a>
            to find the Team Admin options.
        ";
    } else {
        error_page("Foundership request not allowed now");
    }
    break;
case "decline":
    $teamid = post_int("teamid");
    $team = lookup_team($teamid);
    require_founder_login($user, $team);
    page_head("Decline founder change request");
    
    if ($team->ping_user) {
        $ping_user = BoincUser::lookup_id($team->ping_user);
        
        $team->update("ping_user=0");
        send_founder_transfer_decline_email($team, $ping_user);
        echo "<p>The foundership request from ".user_links($ping_user)
            ." has been declined.
        ";
    } else {
        echo "<p>There were no foundership requests.";
    }
    break;
default:
    error_page("undefined action $action");
}

echo "<a href='team_display.php?teamid=$team->id'>Return to team page</a>";

page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
