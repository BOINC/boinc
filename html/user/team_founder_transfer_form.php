<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user(true);
if (!$user->teamid) {
    error_page("You need to be a member of a team to access this page.");
}
$team = lookup_team($user->teamid);

page_head("Transfer founder position of $team->name");
$now = time();

// if founder has declined the request and the request was done more than
// two months ago, allow new request; if both founder and change initiator
// haven't responded for 3 months, allow new request.
//
if (new_transfer_request_ok($team, $now)) {
    echo "<form method=\"post\" action=\"team_founder_transfer_action.php\">";
    echo "<p>If the team founder is no longer active and you feel that you can
        take over from him/her, click the button below. The current team founder
        will be sent an email detailing your request and will be given an option
        to transfer the founder position to you. If the founder does not respond
        in two months, you will be given an option to become the founder
        yourself.
        <p>Are you sure you want to initiate founder transfer process?
    ";

    echo "<input type=\"hidden\" name=\"action\" value=\"transfer\">
        <input type=\"submit\" value=\"Initiate founder transfer process\">
        </form>
    ";
} else {
    if ($team->ping_user) {
        if ($user->id == $team->ping_user) {
            echo "<p>You have already requested to take over the founder
                position of $team->name.
            ";
            if (transfer_ok($team, $now)) {
                echo "<form method=\"post\" action=\"team_founder_transfer_action.php\">
                    <input type=\"hidden\" name=\"action\" value=\"transfer\">
                    <input type=\"submit\" value=\"Complete team founder transfer\">
                    </form>
                ";
            } else {
                echo "<p>The team founder has been notified of your request.
                    If he/she does not respond by ".date_str(transfer_ok_time($team))."
                    you will be given an option to become team founder.
                ";
            }
        } else {
            if ($team->ping_user < 0) {
                $team->ping_user = -$team->ping_user;
            }
            $ping_user = lookup_user_id($team->ping_user);
            echo "<p>Founder change has already been requested by ".
                user_links($ping_user)." on ".date_str($team->ping_time).".
            ";
        }
    } else {
        echo "<p>A founder change has been initiated during the last two
            months and is currently disabled.
        ";
    }
}

echo "<a href=\"team_display.php?teamid=".$team->id."\">Return to team page</a>";

page_tail();

?>
