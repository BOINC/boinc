<?php

require_once("util.inc");
require_once("login.inc");
require_once("team.inc");
db_init();
$id = $HTTP_GET_VARS["id"];

    $query = sprintf(
        "select * from team where id = %d",
        $id
    );
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    $team_name = $team->name;
    $team_id = $team->id;
    page_head("Disband $team_name");
    echo "<h2>Disband $team_name</h2>";
    echo "<table width=780>";
    echo "<tr><td>";
    echo "<p><b>Please note:</b>";
    echo "<ul>";
    echo "<li>Only the found may disband a team.";
    echo "<li>Only teams with no members may be disbanded. The founder of a team ";
    echo "may email the members of a team to inform them of the disbanding of the team.";
    echo "<li>After a team is disbanded, all members will no longer belong to a team, ";
    echo "though they may join another team if desired.";
    echo "<li>Disbanding a team is permanent, make sure you know what you're doing!";
    echo "</ul>";
    echo "</p>";
    echo "Press &quot;Disband Team&quot; when you're ready.";
    echo "<hr>";
    echo "<form method=post action=team_disband_action.php>";
    echo "<input type=hidden name=id value=$team_id>";
    echo "<input type=submit value=\"Disband Team\">";
    echo "</form>";
    echo "</tr></td></table>";
    page_tail();

?>
