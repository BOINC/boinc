<?php

// This is a sample project file.  Replace it with your own.

define("PROJECT", "TestBOINC@home");
define("MASTER_URL", "http://boinc.berkeley.edu/");

function project_intro() {
    echo"
This is a test project of
the Berkeley Open Infrastructure for Network Computing (BOINC)
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
