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
    page_head("User page");
    echo "<h1>$user->name</h1>\n";
    echo "<a href=prefs.php>Preferences</a>";

    show_user($user);
    show_hosts($user);
}
?>
