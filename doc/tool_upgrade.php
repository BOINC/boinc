<?php
   require_once("docutil.php");
   page_head("Upgrading server software");
echo "
<p>
The BOINC server software (scheduler, daemons, web pages)
is continually improved and debugged.
We recommend that projects upgrade to the latest version
every few weeks or so.
There may also be points
upgrades are mandatory to continue working with current client software.

<p>
The steps in upgrading are as follows:

<ol>
<li> (Optional) stop the project,
    and make backups of the project database and the project tree.
<li> <a href=source_code.php>Download</a> (using CVS) the current source code.
Compile it in your BOINC source directory.
<li> 
Run the <code>upgrade</code> script:
<pre>
cd source/tools
upgrade project_name
</pre>

The <code>upgrade</code> script copies files
from the source/html/, source/sched and source/tool
directories to the corresponding project directories
(the default project root directory is \$HOME/projects/project_name;
<code>upgrade</code> takes an optional environment variables INSTALL_DIR
specifying the project's root directory).

<li>
Update your project's database if needed:
<pre>
cd project/html/ops
</pre>
and look at the file db_update.php.
This has a number of functions with names like

<pre>
update_8_05_2005()
</pre>

Each function performs a particular database update.
You must perform all updates, in sequence,
since your last server software upgrade.
(If you're not sure when that was, you can use mysql
to see that current format of your database, e.g.,
to see the fields in the 'user' table, type
<pre>
mysql project_name
&gt; explain user;
</pre>
To do a particular update,
edit db_update.php so that (at the bottom) it calls that function.
Then do
<pre>
php db_update.php
</pre>
Repeat this for the necessary updates, in increasing chronological order.

<li>
Start the project, and check log files to make sure everything is OK.
Run the BOINC client and test basic functions
(attaching to project, getting work).
</ol>

";
page_tail();
?>
