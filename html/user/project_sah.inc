<?php

define("PROJECT", "SETI@home");
define("MASTER_URL", "http://maggie.ssl.berkeley.edu/sah/");

function project_intro() {
    echo"
The SETI@home project, based at UC Berkeley,
uses the idle time of thousands of computers to
search for extraterrestrial intelligence.
";
}

function project_banner($user) {
    echo "<table width=100%><tr><td>
        <img src=http://setiathome.berkeley.edu/images/better_banner.jpg>
        </td><td align=right>\n";
    if ($user) {
        echo "Logged in as $user->name";
    } else {
        echo "<br>";
    }
    echo "</td></tr></table><hr>\n";
}

?>
