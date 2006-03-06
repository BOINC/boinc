<?php
require_once("docutil.php");
page_head("Installing a self-extracting archive (Unix/Linux)");
echo "
BOINC for Unix/Linux is available as a self-extracting archive.
The download files have names like
<pre>
boinc_5.2.13_i686-pc-linux-gnu.sh
</pre>

This type of installation
requires that you be familiar with the
UNIX command-line interface.

<p>
After downloading the file (say, into
        <code>boinc_5.2.13_i686-pc-linux-gnu.sh</code>), type
<pre>
sh boinc_5.2.13_i686-pc-linux-gnu.sh
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
<dd> A script that cd's into the BOINC directory and runs the core client.
<dt>
run_manager
<dd> A script that cd's into the BOINC directory and runs the manager.
</dl>

<p>
The core client has a number of other
<a href=client_unix.php>command-line options</a>.
<p>
You may want to
<a href=auto_start.php>automatically start the core client</a>
at boot time.
<p>
To control a running BOINC client, use the
<a href=boinc_cmd.php>BOINC command tool</a>.


";
page_tail();
?>
