<?php

define("PROJECT", "Astropulse");
define("MASTER_URL", "http://maggie.ssl.berkeley.edu/ap/");
define("URL_BASE", "http://setiboinc.ssl.berkeley.edu/ap");

function project_intro() {
    echo"
        Astropulse uses your computer's idle time to
        analyze radio telescope signals, looking for
        <a href=http://www.planetary.org/html/UPDATES/seti/SETI@home/Update_110501.htm>black hole evaporation</a>
        and other phenomena.
    ";

    echo "<p><b>
        Astropulse is based on <a href=http://boinc.berkeley.edu>BOINC</a>,
        which is being beta-tested.
        We currently have enough volunteers for the beta test.
        Stay tuned for an expanded beta test in the upcoming months.</b>
        <p>
        Beta testers: click <a href=beta_test_instructions.html>here</a>
        for instructions and news.
        ";
}

function project_banner($user) {
    echo "
        <table cellspacing=0 border=0 cellpadding=10 width=100%><tr>
        <td background=http://setiathome.berkeley.edu/images/outrspc3.gif>
        <font color=ffffff size=8 face='helvetica'>Astropulse</font>
        </td>
        <td valign=bottom align=right background=http://setiathome.berkeley.edu/images/outrspc3.gif>
        <font size=-1 color=ffffff>\n
    ";

    if ($user) {
        echo "Logged in as $user->name";
    } else {
        echo "<br>";
    }
    echo "</td></tr></table><br>\n";
}

?>
