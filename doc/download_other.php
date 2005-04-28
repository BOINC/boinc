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
list_heading_array(array(
    "Site", "Platforms", "Programs available"
));
list_item_array(array(
    "<a href=http://www.lb.shuttle.de/apastron/boincDown.shtml>Stefan Urbat</a>",
    "Solaris 10 AMD64 (Opteron) and x86
    <br> GNU/Linux AMD64 (Opteron)
    <br> GNU/Linux PowerPC,
    <br> HPUX,
    <br> Mac OS X 10.3 on PowerPC 7450 and later",
    "BOINC core client, SETI@home"
));

list_item_array(array(
    "<a href=http://members.dslextreme.com/~readerforum/forum_team/boinc.html>
    Team MacNN</a>",
    "Mac OS X, 10.2.8 and 10.3.x, G3/G4/G5",
    "BOINC core client, SETI@home"
));
list_item_array(array(
    "<a href=http://boinc.vawacon.de/>SOLARIS@x86</a>",
    "Solaris 9 on Intel x86",
    "BOINC core client, SETI@home"
));
list_item_array(array(
    "FreeBSD.org",
    "FreeBSD on a variety of hardware.",
    "<a href=http://www.freebsd.org/cgi/ports.cgi?query=boinc-client&stype=all>BOINC core client</a>,
    <a href=http://www.freebsd.org/cgi/ports.cgi?query=boinc-setiathome&stype=all>SETI@home</a>
    "
));

list_end();
page_tail();
?>
