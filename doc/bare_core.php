<?php
require_once("docutil.php");
page_head("Installing the command-line core client");
echo "
This type of installation requires that you be familiar with the
UNIX command-line interface.

<p>
After downloading the file:
<ul>
<li> Use gunzip to uncompress the file
if your browser has not done it for you.
<li> chmod +x the executable.
<li> put the executable into a separate directory (say, boinc/).
<li> run the executable.
    The first time you run it you'll need to attach it to a project, e.g.:
    <pre>
    boinc_client -attach_project http://setiathome.berkeley.edu df0d93e384a2700f70e0bc45a1b2c96c
    </pre>
</ul>
<p>
Instructions on running the core client (command-line options, etc.) are
<a href=client_unix.php>here</a>.
<p>
You'll probably want to arrange
to run the executable each time your machine boots or you log on.
Some examples follow.

<h2> Automatic startup on Mac OS X</h2>
<p>
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
<h2> Automatic startup on Unix</h2>
<p>
Users have contributed scripts for automatic
startup of BOINC on various versions of Unix:

<ul>
<li>
<a href=http://www.spy-hill.net/~myers/help/boinc/unix.html>Red Hat Linux</a>.
(includes general instructions for installing on Unix systems).
<li>
<a href=autostart_unix.txt>Linux with KDE</a>.
<li>
<a href=autostart_dennett.txt>Red Hat 9, should also work on Solaris</a>
<li>
<a href=gentoo.txt>Gentoo Linux</a> (from Gabor Nagy)
<li>
<a href=hpux.html>HPUX</a> (mostly relevant to UNIX in general)
</ul>

";
page_tail();
?>
