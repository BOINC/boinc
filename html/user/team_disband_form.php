<?php

require_once("util.inc");
require_once("team.inc");

    db_init();
    $user = get_logged_in_user();

    $teamid = $_GET["teamid"];

    $result = mysql_query("select * from team where id = $teamid");
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    require_founder_login($user, $team);
    page_head("Disband $team->name");
    echo "<h2>Disband $team->name</h2>
        <b>Please note:</b>
        Only teams with no members may be disbanded.
        You may email the members of the team to ask them to quit.
        <form method=post action=team_disband_action.php>
        <input type=hidden name=id value=$team->id>
        <input type=submit value='Disband team'>
        </form>
    ";
    page_tail();

?>
