<?php

require_once("docutil.php");

page_head("Other sources of BOINC client software");

function site($url, $name, $platforms) {
    echo "<tr><td><a href=$url>$name</a></td><td>$platforms</td></tr>\n";
}

echo "
<p>
The following sites offer downloads of BOINC software
and BOINC project applications, compiled for various platforms.
These downloads are not endorsed by BOINC or any BOINC project;
Use them at your own risk.
    <p>
";
list_start();
echo "<tr><th>Site</th><th>Platforms</th></tr>\n";
list_item(
    "<a href=http://members.dslextreme.com/~readerforum/forum_team/boinc.html>
    Team MacNN</a>",
    "Mac OS X, 10.2.8 and 10.3.x, G3/G4/G5
    <br>(BOINC client and SETI@home app)"
);
list_end();
page_tail();
?>
