<?php

function show_hosts($user) {
    $result = mysql_query("select * from host where userid=$user->id");
    while ($host = mysql_fetch_object($result)) {
        echo "<p>\n";
        show_host($host);
    }
    mysql_free_result($result);
}

function show_user_page($user, $project) {
    echo "<h1>Welcome, $user->name!</h1>\n";
    start_table();
    show_user_profile($user);
    echo "<p><p>\n";
    show_user_stats($user, $project);
    echo "<p>\n";
    echo "<ul>";
    echo "<li><a href=show_hosts.php>Host info</a></li>";
    echo "<li><a href=prefs.php>Preferences</a></li>";
    echo "<li><a href=team.php>Teams</a></li>";
    echo "</ul>";

}
?>
