<?php {

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user(true);
$id = $user->teamid;

$result = mysql_query("select * from team where id=$id");
if ($result) {
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
}
$team_name = $team->name;
$team_id = $team->id;
page_head("Quit $team_name");
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
    <input type=hidden name=id value=$team_id>
    <input type=submit value=\"Quit Team\">
    </form>
";
page_tail();

} ?>
