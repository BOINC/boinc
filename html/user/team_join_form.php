<?php

require_once("util.inc");
require_once("team.inc");

db_init();
$id = $HTTP_GET_VARS["id"];

    $query = "select * from team where id = $id";
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    $team_name = $team->name;
    $team_id = $team->id;
    page_head("Join $team_name");
    echo "<h2>Join $team_name</h2>";
    echo "<p><b>Please note before joining or switching to a new team:</b>";
    echo "<ul>";
    echo "<li> Your credit will be transferred from your old team (if any)";
    echo " to the new team.";
    echo "<li> Joining a team does not affect your account's credit.";
    echo "<li> Joining a team gives your team's founder access to your email address.";
    echo "</ul>";
    echo "<hr>";
    echo "<form method=post action=team_join_action.php>";
    echo "<input type=hidden name=id value=$team_id>";
    echo "<input type=submit value=\"Join Team\">";
    echo "</form>";
    page_tail();

?>
