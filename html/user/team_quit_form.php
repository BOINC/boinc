<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user(true);

$team = lookup_team($user->teamid);
if (!$team) {
    error_page("No such team");
}

page_head("Quit $team->name");
echo "
    <b>Please note before quitting a team:</b>
    <ul>
    <li>If you quit a team, you may rejoin later,
    or join any other team you desire
    <li>Quitting a team does not affect your personal credit
    statistics in any way.
    </ul>
    </p>
    <form method=post action=team_quit_action.php>
    <input type=hidden name=id value=$team->id>
    <input type=submit value=\"Quit Team\">
    </form>
";
page_tail();

?>
