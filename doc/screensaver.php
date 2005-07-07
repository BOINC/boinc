<?php
require_once("docutil.php");
page_head("The BOINC screensaver");
echo "
The BOINC client software on Windows and Mac OS X includes a screensaver program,
which you can select as your screensaver
(on Windows, right-click on the desktop,
click Properties, select Screen Saver, select BOINC).
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
and their fraction done.

<li> If the core client is running but no applications are running,
the screensaver shows that.

<li> If the core client is not running,
    the screensaver shows that.
</ul>
<p>
Normally, if you press a key or mouse the mouse
while the screensaver is active,
the screensaver will exit.
Some BOINC applications handle keyboard/mouse input.
If you hold down the control key,
any additional input will be passed to the application,
and your computer will remain in screensaver mode.
";
page_tail();
?>
