<?php
require_once("docutil.php");
page_head("Project creation cookbook");
echo "
<h2>Make skeletal project</h2>
<ul>
<li> Compile the BOINC software, say into HOME/boinc.
Make sure MySQL is configured and running.
<li> Run HOME/boinc/tools/<a href=make_project.php>make_project</a>
<li> Append the contents of projects/PROJECT/PROJECT.httpd.conf
to httpd.conf and restart Apache.
<li> Use 'crontab' to insert a cron job to run the project's
periodic tasks, e.g.
    <br> 0,5,10,15,20,25,30,35,40,45,50,55 * * * * HOME/projects/PROJECT/bin/start --cron
    <br> (if cron cannot run 'start', try using a helper script
    to set PATH and PYTHONPATH)

<li> Copy project.xml from HOME/boinc/tools to HOME/projects/PROJECT,
edit it to taste,
and run <a href=tool_xadd.php>bin/xadd</a>.
<li> Edit html/project/project.inc, changing the
master URL and copyright holder.
<li> Protect the html/ops directory
(e.g. by putting .htaccess and .htpasswd files there).
</ul>

Visible result: the project web site is up.
The database 'platforms' table has several rows.
<p>
Troubleshooting:
check the Apache access and error logs.

<h2>Create an application version</h2>
<ul>
<li> Create a BOINC application executable
(if you're in a hurry, use the test application).
<li> Copy the executable to HOME/projects/PROJECTNAME/apps/APPNAME
<li> cd to HOME/projects/PROJECTNAME/bin
<li> run ./update_versions, type y or return.
<li> run ./stop && ./start
</ul>

Visible result: the web site's Applications page has an entry.

<h2>Create a work unit</h2>
<ul>
<li> Using a text editor, create a work unit template file
and a result template file.
<li> Run create_work
<li> Edit config.xml to add
".htmlspecialchars("<daemon>")." records for
make_work,
feeder,
transitioner,
file_deleter,
the trivial validator,
and the trivial assimilator.
For example",
html_text(
"<daemon>
    <cmd>validate_test -app appname</cmd>
    <output>validate_test.log</output>
    <pid_file>validate_test.pid</pid_file>
</daemon>"
),
"
</ul>

Visible result: after a project restart,
'status' shows the above daemon processes running.
<p>
Troubleshooting: check the log files of all daemon processes.

<h2>Test the system</h2>
<ul>
<li> Create a client directory (on the same computer or different computer), say HOME/boinc_client.
Copy the core client there.
<li> Using the web interface, create an account on the project.
<li> Run the core client;
enter the project URL and the account key.
</ul>

Visible result: the client does a stream of work;
the web site shows credit accumulating.

<p>
Troubleshooting: check the log files of all daemon processes.

<h2>Develop back end components</h2>
<ul>
<li> Write a work generator.
<li> Write a validator.
<li> Write an assimilator.
<li> Edit the configuration file to use these programs
instead of the place-holder programs.
<li> Make sure everything works correctly.
</ul>


<h2>Extras</h2>

<ul>
<li> Make the core client available from your site
<li> Add message board categories: see html/ops/create_forums.php
</ul>

";

page_tail();
?>
