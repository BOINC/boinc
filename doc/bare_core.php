<?php
require_once("docutil.php");
page_head("Installing the core client");
echo "
This type of installation
requires that you be familiar with the
UNIX command-line interface.

<p>
After downloading the file:
<ul>
<li> Use gunzip to uncompress the file
if your browser has not done it for you.
<li> chmod +x the executable.
<li> put the executable into a separate directory (say, boinc/).
<li> run the executable.
</ul>
You will need to develop your own mechanism
to run the executable each time your machine boots
or you log on.

";
page_tail();
?>
