<?php
require_once("docutil.php");
page_head("Installing a self-extracting archive");
echo "
This type of installation
requires that you be familiar with the
UNIX command-line interface.

<p>
After downloading the file (say, into file X):
<ul>
<li> put the file into a separate directory (say, boinc/).
<li> type sh < X
<li> this will create two files,
  the core client and the BOINC manager.
<li> run the core client.
If you like, run the BOINC manager.
</ul>
You will need to develop your own mechanism
to run the executable each time your machine boots
or you log on.

";
page_tail();
?>
