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
Read <a href=bare_core.php>Installing the command-line client</a>.
<p>
";
list_start();
list_heading_array(array(
    "Site", "Platforms", "Programs available"
));
list_item_array(array(
    "<a href=http://naparst.name/>Harold Naparst</a>",
    "Linux/x86",
    "SETI@home application"
));
list_item_array(array(
    "<a href=http://www.godefroy.t.freesurf.fr/seti/>SETI@home and BOINC on Linux</a>",
    "Mandrake Linux",
    "BOINC and SETI@home (graphical versions)"
));
list_item_array(array(
    "<a href=http://forums.macnn.com/showthread.php?t=266339>macnn.com</a>",
    "Mac OS X",
    "SETI@home app"
));
list_item_array(array(
    "<a href=http://rcswww.urz.tu-dresden.de/~s3240790>Erik Trauschke</a>",
    "Irix",
    "BOINC core client and SETI@home app"
));
list_item_array(array(
    "<a href=http://interreality.org/~reed/sw/boinc/>http://interreality.org/~reed/sw/boinc/</a>",
    "Debian Linux",
    "BOINC client and manager"
));
list_item_array(array(
    "SETI@BOINC (<a href=http://www.marisan.nl/seti/>English</a>,
    <a href=http://www.marisan.nl/seti/index_nl.htm>Dutch</a>)",
    "Windows",
    "SETI@home"
));
list_item_array(array(
    "<a href=http://www.pperry.f2s.com/downloads.htm>SETI-Linux</a>",
    "Linux i686, linux athlon xp, Linux AMD64,
    Linux Pentium 3. Some Links to other Platforms",
    "BOINC, SETI@home"
));
list_item_array(array(
    "<a href=http://pkg-boinc.alioth.debian.org/binary/>Debian.org</a>",
    "Debian Linux on x86",
    "BOINC core client and manager"
));
list_item_array(array(
    "<a href=http://www.guntec.de/Crunch3r>Matthias Pilch</a>",
    "Linux on DEC, IA64",
    "BOINC, SETI@home"
));
list_item_array(array(
    "<a href=http://www.kulthea.net/boinc/>SETI@Kulthea.net</a>",
    "Linux on Sparc",
    "BOINC, SETI@home"
));
list_item_array(array(
    "<a href=http://www.lb.shuttle.de/apastron/boincDown.shtml>Stefan Urbat</a>",
    "Solaris 10 AMD64 (Opteron) and x86
    <br> GNU/Linux AMD64 (Opteron)
    <br> GNU/Linux PowerPC,
    <br> Linux Itanium
    <br> HPUX (PA RISC and Itanium/IA64),
    <br> Tru64@Alpha,
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
echo "
If you have a download server not listed here,
please send email to davea at ssl.berkeley.edu.
";
page_tail();
?>
