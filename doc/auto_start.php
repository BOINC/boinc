<?php

require_once("docutil.php");

page_head("Starting BOINC automatically");

echo "
<p>
You'll probably want to arrange
to run the executable each time your machine boots or you log on.
Some examples follow.
<h2> Automatic startup on Unix</h2>
<p>
Users have contributed scripts for automatic
startup of BOINC on various versions of Unix:

<ul>
<li>
<a href=http://www.spy-hill.net/~myers/help/boinc/unix.html>Red Hat Linux</a>.
(includes general instructions for installing on Unix systems).
<li>
<a href=http://alioth.debian.org/projects/pkg-boinc/>BOINC on Debian Linux</a>.
<li>
<a href=autostart_unix.txt>Linux with KDE</a>.
<li>
<a href=autostart_dennett.txt>Red Hat 9, should also work on Solaris</a>
<li>
<a href=gentoo.txt>Gentoo Linux</a> (from Gabor Nagy)
<li>
<a href=hpux.html>HPUX</a> (mostly relevant to UNIX in general)
<li> <a href=http://rigo.altervista.org/tools/>An init script for running BOINC under SuSE Linux (9.2 and 9.3)</a>
</ul>

<h2> Automatic startup on Mac OS X</h2>
<p>
Note: the Mac Standard GUI installation arranges
for BOINC to run on system startup.
The following is relevant if you install the command-line client.
<ul>
<li>
Instructions from Paul Buck's
<a href=http://boinc-doc.net/site-boinc/oman-app/inst-mac.php>BOINC Owner's Manual</a>.
<li>
<a href=mac_yenigun.php>Instructions from Berki Yenigun</a>,
deadsmile at minitel.net
<li>
<a href=http://stegic.net/archives/2005/05/boinc_as_a_syst.html>Running
BOINC as a system service on Mac OS X</a> by Ivan Stegic.
</ul>
";

page_tail();
?>
