<?php

// This is a sample project file.  Replace it with your own.

define("PROJECT", "Astropulse");
define("MASTER_URL", "http://maggie.ssl.berkeley.edu/ap/");

function project_intro() {
    echo"
The Astropulse project, based at UC Berkeley,
uses the idle time of thousands of computers to
analyze radio telescope signals.
";
}

function project_banner($user) {
    // Put your project title and logo here

    echo "<table width=100%><tr><td><h2>".PROJECT."</h2></td><td align=right>\n";
    if ($user) {
        echo "Logged in as $user->name";
    } else {
        echo "<br>";
    }
    echo "</td></tr></table><hr>\n";
}

?>
