<?php
   require_once("docutil.php");
   page_head("Upgrading project software");
echo "
<p>
The <code>upgrade</code> script copies files from a BOINC source tree to a
project tree.
This is typically done after updating the source tree and
recompiling the software.

<p>
Typical usage:
<pre>
cd source/tools
upgrade project_name
</pre>

<p>
<code>upgrade</code> does the following:
<ul>
<li> Stop the project.
<li>
Copy files from the source/html/* directories to the corresponding
project directories
(the default project root directory is \$HOME/projects/project_name).
<li>
Copy source/sched/cgi and source/sched/file_upload_handler to
project_name/cgi-bin.
<li>
Copy other programs in source/sched to project_name/bin.
<li>
Copy programs in source/tools to project_name/bin.
<li> Remind you to restart the project.
</ul>

<p>
WARNING: some changes in the BOINC server software
require corresponding changes to your BOINC database.
Check the file html/ops/db_update.php
and apply whatever changes are needed before restarting the project.
<p>
<code>upgrade</code> takes an optional environment variables INSTALL_DIR
specifying the project's root directory.

";
page_tail();
?>
