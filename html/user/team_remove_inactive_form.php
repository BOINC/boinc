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
    $nusers = $team->nusers;
    page_head("Remove Members from $team_name");
    echo "<h2>Remove members from $team_name</h2>";
    echo "<table width=780>";
    echo "<tr><td>";
    echo "<p><b>Please note:</b>";
    echo "<ul>";
    echo "<li>Only the founder may remove members from a team";
    echo "<li>By removing a member, you will also remove their credit and CPU time ";
    echo "contributions to the team.";
    echo "</ul>";
    echo "</p>";
    echo "<hr>";
    echo "<form method=post action=team_remove_inactive_action.php>";
    echo "<input type=hidden name=id value=$team_id>";
    echo "<br></td></tr></table>";
    echo "<table border=1><tr><th>Remove?</th>";
    echo "<th>Name</th>";
    echo "<th>Total<br>Credit</th>";

    $query = sprintf(
        "select * from user where teamid = %d",
        $team_id
    );
    $result = mysql_query($query);

    $ninactive_users = 0;
    for ($i = 0; $i < $nusers; $i++) {
        $user = mysql_fetch_object($result);
        echo "<tr><td align=center><input type=checkbox name=remove_$ninactive_users value=$user->id>";
        echo "<td>$user->name</td>";
        echo "<td>$user->total_credit</td>";
        $ninactive_users++;
    }
    echo "<input type=hidden name=ninactive_users value=$ninactive_users>";
    if ($result) {
       mysql_free_result($result);
    }
    echo "</table>";
    echo "<input type=submit value=\"Remove Users\">";
    echo "</form>";
    page_tail();
?>
