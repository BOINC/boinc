<?php

require_once("docutil.php");

page_head("Other sources of BOINC client software");

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
$site_list = array(
array(
    "<a href=http://www.mikrocomp.hu/seti/>SETI@SummoneR.mikrocomp.hu</a>",
    "OpenBSD: amd64 (x86_64)",
    "BOINC core client and SETI@home Enhanced non-graphical"
),
array(
    "<a href=http://lunatics.at>lunatics.at</a>",
    "Linux and Windows/x86",
    "BOINC and SETI@home.
    Also, <a href=http://lunatics.at/index.php?op=Articles>How-Tos
    for BOINC and SETI@home compilation for Linux and Windows</a>."
),
array(
    "<a href=http://boinc.truxoft.com/>boinc.truxoft.com</a>",
    "Windows/Intel",
    "BOINC client (optimized; benchmarks more closely
    match optimized SETI@home clients)"
),
array(
    "<a href=http://naparst.name/>Harold Naparst</a>",
    "Linux/x86",
    "SETI@home application"
),
array(
    "<a href=http://www.godefroy.t.freesurf.fr/seti/>SETI@home and BOINC on Linux</a>",
    "Mandrake Linux",
    "BOINC and SETI@home (graphical versions)"
),
array(
    "<a href=http://forums.macnn.com/showthread.php?t=266339>macnn.com</a>",
    "Mac OS X",
    "SETI@home app"
),
array(
    "<a href=http://www.nekochan.net/nekoware/index.php?path=current/>nekochan.net</a>",
    "SGI Irix 6.5.22 and above",
    "<a href=http://www.nekochan.net/nekoware/current/neko_boinc_client-5.4.9.tardist>BOINC client 5.4.9</a>
    and <a href=http://www.nekochan.net/nekoware/current/neko_setiathome_enhanced-5.12.tardist>SETI@home enhanced 5.12</a>."
),
//array(
//    "<a href=http://rcswww.urz.tu-dresden.de/~s3240790>Erik Trauschke</a>",
//    "Irix",
//    "BOINC core client and SETI@home app"
//),
array(
    "SETI@BOINC (<a href=http://www.marisan.nl/seti/>English</a>,
    <a href=http://www.marisan.nl/seti/index_nl.htm>Dutch</a>)",
    "Windows",
    "SETI@home"
),
array(
    "<a href=http://www.pperry.f2s.com/downloads.htm>SETI-Linux</a>",
    "Linux: i686, Athlon-XP, AMD64, Pentium 3. Some Links to other Platforms",
    "BOINC, SETI@home"
),
array(
    "
    <a href=http://alioth.debian.org/projects/pkg-boinc/>alioth.debian.org</a>;
    installation instructions at
    <a href=http://wiki.debian.org/BOINC#Installation>wiki.debian.org</a>",
    "Debian Linux on alpha, amd64, arm, hppa, 
    i386, ia64, kfreebsd-i386, m68k, mips, mipsel, powerpc, s390 and sparc",
     "<a href=http://packages.debian.org/boinc-client>BOINC core client</a>,
     <a href=http://packages.debian.org/boinc-manager>BOINC manager</a>
     and <a href=http://packages.debian.org/boinc-app-seti>SETI@home</a>"
),
array(
    "<a href=http://calbe.dw70.de>Matthias Pilch</a>",
    "Windows, Linux on DEC, IA64, FreeBSD",
    "BOINC, SETI@home, SETI@home Enhanced"
),
array(
    "<a href=http://www.kulthea.net/boinc/>SETI@Kulthea.net</a>",
    "Linux on Sparc64",
    "BOINC, SETI@home"
),
array(
    "<a href=http://www.lb.shuttle.de/apastron/boincDown.shtml>Stefan Urbat</a>",
    "Solaris: SPARC, AMD64 (Opteron), x86, i686
    <br> Linux: Alpha, SPARC, AMD64 (Opteron), PowerPC, Itanium, s390
    <br> HP-UX: PA RISC and Itanium/IA64 (includes BOINC Manager)
    <br> Tru64: Alpha
    <br> AIX: Power4 and later
    <br> OpenBSD: x86
    <br> NetBSD: x86
    <br> FreeBSD: x86, Alpha
    ",
    "BOINC core client, SETI@home"
),

array(
    "<a href=http://members.dslextreme.com/~readerforum/forum_team/boinc.html>
    Team MacNN</a>",
    "Mac OS X, 10.2.8 and 10.3.x, G3/G4/G5",
    "BOINC core client, SETI@home"
),
array(
    "<a href=http://boinc.vawacon.de/>SOLARIS@x86</a>",
    "Solaris 9 on Intel x86",
    "BOINC core client, SETI@home"
),
array(
    "FreeBSD.org",
    "FreeBSD on a variety of hardware.",
    "<a href=http://www.freebsd.org/cgi/ports.cgi?query=boinc-client&stype=all>BOINC core client</a>,
    <a href=http://www.freebsd.org/cgi/ports.cgi?query=boinc-setiathome&stype=all>SETI@home</a>
    "
),
);

list_start();
list_heading_array(array(
    "Site", "Platforms", "Programs available"
));
shuffle($site_list);
foreach ($site_list as $site) {
    list_item_array($site);
}
list_end();
echo "
If you have a download server not listed here,
please send email to davea at ssl.berkeley.edu.
";
page_tail();
?>
