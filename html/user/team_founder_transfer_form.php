<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

$user = get_logged_in_user();
$team = BoincTeam::lookup_id($user->teamid);
if (!$team) {
    error_page("You need to be a member of a team to access this page.");
}

page_head("Request foundership of $team->name");
$now = time();

if (new_transfer_request_ok($team, $now)) {
    echo "<form method=\"post\" action=\"team_founder_transfer_action.php\">";
    echo "<p>If the team founder is not active and you want to assume
        the role of founder, click the button below.
        The current founder will be sent an email detailing your request,
        and will be given an option to transfer foundership to you
        or to decline your request.
        If the founder does not respond in 60 days,
        you will be given an option to become the founder.
        <p>
        Are you sure you want to request foundership?
    ";

    echo "<input type=\"hidden\" name=\"action\" value=\"transfer\">
        <input type=\"submit\" value=\"Request foundership\">
        </form>
    ";
} else {
    if ($team->ping_user) {
        if ($user->id == $team->ping_user) {
            echo "<p>You have already requested the foundership
                of $team->name.
            ";
            if (transfer_ok($team, $now)) {
                echo "
                    60 days have elapsed since your request,
                    and the founder has not responded.
                    You may now assume foundership by clicking here:
                    <form method=\"post\" action=\"team_founder_transfer_action.php\">
                    <input type=\"hidden\" name=\"action\" value=\"transfer\">
                    <input type=\"submit\" value=\"Assume foundership\">
                    </form>
                ";
            } else {
                echo "<p>
                    The founder has been notified of your request.
                    If he/she does not respond by ".date_str(transfer_ok_time($team))."
                    you will be given an option to become founder.
                ";
            }
        } else {
            if ($team->ping_user < 0) {
                $team->ping_user = -$team->ping_user;
            }
            $ping_user = BoincUser::lookup_id($team->ping_user);
            echo "<p>Founder change has already been requested by ".
                user_links($ping_user)." on ".date_str($team->ping_time).".
            ";
        }
    } else {
        echo "<p>A foundership change was requested during the last 90 days,
             so new requests are not allowed.
             Please try again later.
        ";
    }
}

echo "<p><a href=\"team_display.php?teamid=".$team->id."\">Return to team page</a>";

page_tail();

?>
