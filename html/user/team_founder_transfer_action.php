<?php

// action = 'transfer':
//    handle a user's request to initiate a foundership transfer 
// action = 'decline':
//    handle the current founder's declining of the request

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");

$user = get_logged_in_user();
if (!$user->teamid) {
    error_page("You must be a member of a team to access this page.");
}

function send_founder_transfer_email($team, $user) {
    $body = "Team member ".$user->name." has asked that you
transfer foundership of $team->name in ".PROJECT.".
Please visit
".URL_BASE."/team_change_founder_form.php?teamid=".$team->id."
to transfer foundership or decline the request.
    
If you do not respond to this request within two months, ".$user->name." will
be given the option to become the team founder.
    
Please do not respond to this email.
The mailbox is not monitored and the email
was sent using an automated system.";
    
    $founder = lookup_user_id($team->userid);

    return send_email($founder, PROJECT." team founder transfer", $body);
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

if ($action == "transfer") {
    $team = BoincTeam::lookup_id($user->teamid);
    page_head("Request foundership of ".$team->name);
    $now = time();

    if (new_transfer_request_ok($team, $now)) {
        $success = send_founder_transfer_email($team, $user);
        if ($success) {
            $team->update("ping_user=$user->id, ping_time=$now");
            echo "<p>
                The current founder has been notified of your request by email.
                <p>
                If the founder does not respond within 60 days you will be
                allowed to become the founder.
            ";
        } else {
            echo "Couldn't send notification email; please try again later.";
        }
    } else {
        if ($team->ping_user) {
            if ($user->id == $team->ping_user) {
                if (transfer_ok($team, $now)) {
                    $team->update("userid=$user->id");
                    echo "<p>Congratulations, you are now the new founder of team ".$team->name."
                    Go to <a href=\"".URL_BASE."/home.php\">Your Account page</a>
                    to find the Team Admin options.";
                } else {
                    echo "<p>
                        You have already requested the foundership
                        of $team->name.
                        <p>
                        Team founder has been notified about your request.
                        If he/she does not respond by ".time_str(transfer_ok_time($team))."
                        you will be given the option to assume team foundership.
                    ";
                }
            } else {
                $ping_user = lookup_user_id($team->ping_user);
                echo "<p>Foundership was requested by ".user_links($ping_user)." on ".time_str($team->ping_time);
            }
        } else {
            echo "<p>A foundership change has been requested in the last three
                months and new requests are currently disabled.
            ";
        }
    }
} else if ($action == "decline") {
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
} else {
    error_page("no action");
}

echo "<a href='team_display.php?teamid=$team->id'>Return to team page</a>";

page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
