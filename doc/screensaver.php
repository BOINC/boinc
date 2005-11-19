<?php
require_once("docutil.php");
page_head("The BOINC screensaver");
echo "
The BOINC client software on Windows and Mac OS X includes a screensaver program,
which you can select as your screensaver.
<p>
On Windows:
    <ul>
    <li>Right-click on the desktop.
    <li>Click <b>Properties</b>, select <b>Screen Saver</b>, select <b>BOINC</b>.
    </ul>
On Mac OS X:
    <ul>
    <li>Under the Apple menu, select <b>System Preferences</b>.
    <li>Click on <b>Desktop & Screen Saver</b>.
    <li>Select <b>BOINCSaver</b> from the list of Screen Savers.
    </ul>
<p>
NOTE: BOINC runs even if you don't use the screensaver.
<p>
When active, the screensaver does the following:

<ul>
<li> If a graphics-capable application is running,
the screensaver causes that application to
provide full-screen graphics.
If more than one is running (e.g. on a multiprocessor)
the screensaver cycles between them every few minutes.

<li> If applications are running but non are graphics-capable,
the screensaver shows the names of the applications
and their fraction done for the current work unit.

<li> If the core client is running but no applications are running,
the screensaver shows that.

<li> If the core client is not running,
    the screensaver shows that.
</ul>
<p>
Normally, if you press a key or move the mouse
while the screensaver is active,
the screensaver will exit.
Some BOINC applications handle keyboard/mouse input.
If you hold down the control key,
any additional input will be passed to the application,
and your computer will remain in screensaver mode.
";
page_tail();
?>
