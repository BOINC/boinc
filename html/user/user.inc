<?php

function show_hosts($user) {
    $i=0;
    $result = mysql_query("select * from host where userid=$user->id");
    while ($host = mysql_fetch_object($result)) {
        echo "<br><br>Host $i:\n";
        show_host($host);
        $i++;
    }
    mysql_free_result($result);
}

function show_user_page($user) {
    echo "<h1>Welcome $user->name</h1>\n";
    start_table();
    row("Total credit", $user->total_credit);
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row("Team", "<a href=team_display.php?id=$team->id>$team->name</a>");
    } else {
        row("Team", "none");
    }
    echo "</table>";
    echo "<p>\n";
    echo "<ul>";
    echo "<li><a href=show_user.php>Complete user info</a></li>";
    echo "<li><a href=show_hosts.php>Host info</a></li>";
    echo "<li><a href=prefs.php>Preferences</a></li>";
    echo "<li><a href=team.php>Teams</a></li>";
    echo "</ul>";

}
?>
