<?
require_once("docutil.php");
page_head("Projection creation cookbook");
echo "
<h2>Make skeletal project</h2>
<ul>
<li> Compile the BOINC software, say into HOME/boinc.
<li> Create a directory <b>projects</b> in your home directory
<li> cd into HOME/boinc/tools
<li> run make_project
<li> paste text into httpd.conf
<li> insert cron job
<li> xadd project.xml
<li> Edit html/project/project.inc to use the right
master URL and copyright holder.
<li> Add .htaccess and .htpasswd files to html/ops.
</ul>

Visible result: the project web site is up.

<h2>Create an application version</h2>
<ul>
<li> Create a BOINC application executable
(if you're in a hurry, use the test application).
<li> Copy the executable to HOME/projects/PROJECTNAME>/apps/APPNAME
<li> cd to HOME/projects/PROJECTNAME/bin
<li> run update_versions, type y or return.
<li> cd to HOME/projects/PROJECTNAME
<li> type bin/stop, then bin/start
</ul>

Visible result: the web site's Applications page has an entry.

<h2>Create a work unit</h2>
<ul>
<li> Using a text editor, create a work unit template file
and a result template file.
<li> Run add_work
<li> Edit the configuration file to
add make_work,
the feeder,
the transitioner,
the file deleter,
the trivial validator,
and the trivial assimilator.
</ul>

Visible result: 'status' shows everything running.

<h2>Test the system</h2>
<ul>
<li> Create a client directory, say HOME/boinc_client.
Copy the core client there.
<li> Using the web interface, create an account on the project.
<li> Run the core client;
enter the project URL and the account ID.
</ul>

Visible result: the client does a stream of work;
the web site shows credit accumulating.

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
