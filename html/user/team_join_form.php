<?php

require_once("util.inc");
require_once("team.inc");

db_init();
$user = get_logged_in_user();
$id = $_GET["id"];

    $query = "select * from team where id = $id";
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    $team_name = $team->name;
    $team_id = $team->id;
    page_head("Join $team_name");
    echo "<h2>Join $team_name</h2>
        <p><b>Please note before joining or switching to a team:</b>
        <ul>
        <li> Your credit will be transferred from your old team (if any)
         to the new team.
        <li> Joining a team does not affect your account's credit.
        <li> Joining a team gives its founder access to your email address.
        </ul>
        <hr>
        <form method=post action=team_join_action.php>
        <input type=hidden name=teamid value=$team_id>
        <input type=submit value='Join team'>
        </form>
    ";
    page_tail();

?>
