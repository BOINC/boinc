<?php

define("PROJECT", "Astropulse");
define("MASTER_URL", "http://maggie.ssl.berkeley.edu/apt/");

function project_intro() {
    echo"
The Astropulse project, based at UC Berkeley,
uses the idle time of thousands of computers to
analyze radio telescope signals.
";
}

function project_banner($user) {
    echo "<table cellspacing=0 border=0 cellpadding=10 width=100%><tr>
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
