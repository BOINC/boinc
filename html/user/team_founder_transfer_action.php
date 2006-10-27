<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");

db_init();
$user = get_logged_in_user(true);

$action = post_str("action");

if ($action == "transfer") {
    $team = lookup_team($user->teamid);
    page_head("Take over founder position of ".$team->name);
    $now = time();

    if ((($team->ping_user == 0) && ($team->ping_time < $now - 60 * 86400)) || 
	($team->ping_time < $now - 90 * 86400)) {

	mysql_query("UPDATE team SET ping_user=".$user->id.", ping_time=".$now." WHERE id=".$team->id);
	send_founder_transfer_email($team, $user);
    
	echo "<p>An email with your request to transfer the team to you has been
	    sent to the team founder.</p>
	    <p>If the team founder does not react within two months you will be
	    given an option to become the founder yourself.</p>
	";
    
    } else {
	if ($team->ping_user) {
	    if ($user->id == $team->ping_user) {
		if ($team->ping_time > $now - 60 * 86400) {
		    echo "<p>You have already requested to change the founder
			position of $team->name.</p>
			<p>Team founder has been notified about your request.
			If he/she does not respond until ".date_str($team->ping_time+60*86400)."
			you will be given an option to take over the team founder
			position.</p>";
		} else {
		    mysql_query("UPDATE team SET userid=".$user->id." WHERE id=".$team->id);
		    echo "<p>Congratulations, you are now the new founder of team ".$team->name."
			Go to <a href=\"".URL_BASE."/home.php\">Your Account page</a>
			to find the Team Management options.</p>";
		}
	    } else {
		$ping_user = lookup_user_id($team->ping_user);
		echo "<p>Founder change has already been initiated by ".
		    user_links($ping_user)." on ".date_str($team->ping_time)."</p>";
	    }
	} else {
	    echo "<p>Founder change has already been initiated in the last three
		months and is currently disabled.</p>";
	}
    }
} else if ($action == "decline") {
    $teamid = post_int("teamid");
    $team = lookup_team($teamid);
    require_founder_login($user, $team);
    page_head("Decline founder change request");
    
    if ($team->ping_user) {
	$ping_user = lookup_user_id($team->ping_user);
    
	mysql_query("UPDATE team SET ping_user=0 WHERE id=".$team->id);
	send_founder_transfer_decline_email($team, $ping_user);
	echo "<p>Team founder change from team member ".user_links($ping_user)
	    ." has been declined.</p>
	";
    } else {
	echo "<p>There were no founder change requests at this time.</p>";
    }
}

echo "<a href='team_display.php?teamid=$team->id'>Return to team page</a>";

page_tail();

?>
