<?php
require_once("docutil.php");
page_head("Installing a self-extracting archive");
echo "
This type of installation
requires that you be familiar with the
UNIX command-line interface.

<p>
After downloading the file (say, into file X), type
<pre>
sh X
</pre>
This will create a directory BOINC/
with the following files:
<dl>
<dt> boinc
<dd> The BOINC core client
<dt> boincmgr
<dd> The BOINC manager
<dt>
run_client
<dd> A script the cd's into the BOINC directory
and runs the core client.
</dl>

You may want to run the executable each time your machine boots
or you log on.
Information on this is <a href=bare_core.php>here</a>.

";
page_tail();
?>
