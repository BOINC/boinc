<?php

require_once("util.inc");
require_once("team.inc");
$authenticator = init_session();
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
    page_head("Quit $team_name");
    echo "<h2>Quit $team_name</h2>";
    echo "<table width=780>";
    echo "<tr><td>";
    echo "<p><b>Please note before quitting a team:</b>";
    echo "<ul>";
    echo "<li>By quitting a team you remove your name from the team listing.  ";
    echo "Your credit contribution to the team is also removed";
    echo "<li>Even if you quit a team, you may rejoin later, ";
    echo "or join any other team you desire ";
    echo "<li>Quitting a team does not affect your personal credit ";
    echo "statistics in any way.";
    echo "</ul>";
    echo "</p>";
    echo "<hr>";
    echo "<form method=post action=team_quit_action.php>";
    echo "<input type=hidden name=id value=$team_id>";
    echo "<input type=submit value=\"Quit Team\">";
    echo "</form>";
    echo "</td></tr></table>";
    page_tail();

?>
