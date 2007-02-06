<?php

require_once("docutil.php");

page_head("Increasing server capacity");

echo"
The BOINC server software is designed so that a project
with tens of thousands of volunteers can run on a single server computer.
However, the capacity of this computer may eventually be exceeded.
Symptoms of server overload include:
<ul>
<li> dropped connections
<li> slow web site access
<li> daemons fall behind
<li> database queries take minutes or hours to complete.
</ul>
BOINC has a scalable server architecture that lets
you increase server capacity by adding more computers.
The steps are as follows.

<h3>Run MySQL on a separate host</h3>
When you initially create a BOINC project using
<a href=make_project.php>make_project</a>,
everything runs on a single host:
MySQL database server,
web server, scheduling server, daemons, tasks,
and file upload handler.

<p>
Of these tasks, the MySQL server does the most work
(typically as much as all the others combined).
So, if you need to increase the capacity of your server
the first step is to move the MySQL server to a separate host
(preferably a fast computer with lots of memory).
Specify this host in the
<a href=project_options.php>project configuration file</a>.

<h3>Run server daemons and tasks on multiple hosts</h3>
<p>
If you need more server capacity,
you can move some of the server <a href=project_daemons.php>daemons</a>
and <a href=project_tasks.php>tasks</a> to separate hosts.
(Start by moving the one that's doing the most work).
<p>
When you move daemons and tasks to other hosts,
those hosts must satisfy the following rules:

<ul>
<li> The <a href=groups.php>project admin account</a>
must exist on all hosts,
and a user must be able to use 'ssh' to run
commands on any other host without typing a password.

<li> The hosts must share a common network file system,
and path to the project directory relative to the
project admin's home directory (typically ~/projects/PROJECT_NAME)
must be the same on all hosts.

<li> One host is designated as the project's <b>main host</b>
in config.xml.
<b>The 'start', 'stop', and 'status' scripts should normally
be run on the main host</b>
(if you run them on a different host X,
they'll affect only daemons and tasks on host X).

<li> The project admin account on all hosts must be
able to access the project's MySQL database.
(Exception: data servers and file upload handlers don't need DB access).
</ul>

Host locations are specified as follows:
<ul>
<li> Scheduling servers are listed in the project's
<a href=server_components.php>master page</a>.
<li> The hosts on which tasks and daemons are run
are specified in the <a href=configuration.php>config.xml</a> file.
<li> Data servers are listed in
<a href=tools_work.php#wu_template>workunit template files</a>.
<li> File upload handlers are listed in
<a href=tools_work.php#result_template>result template files</a>.
<li> Your web server runs on the host to which your project URL is mapped.
</ul>
<h3>Parallelize schedulers and daemons</h3>
If you need even more server capacity,
you can parallelize most of the BOINC daemons
so that multiple instances run on a single (multiprocessor) host,
or on different hosts.
For example, the following
<a href=project_daemons.php>config.xml</a> entries
run two instances of the transitioner on the host 'kosh'.
Because these instances are on the same host,
you must specify different output and PID files.
".html_text("
<daemon>
  <host>kosh</host>
  <cmd>transitioner -d 1 -mod 2 0</cmd>
  <output>transitioner0.log</output>
  <pid_file>transitioner0.pid</pid_file>
</daemon>
<daemon>
  <host>kosh</host>
  <cmd>transitioner -d 1 -mod 2 1</cmd>
  <output>transitioner1.log</output>
  <pid_file>transitioner1.pid</pid_file>
</daemon>
")."
You can run scheduling servers on multiple hosts
by running an instance of the feeder on each host,
and including the URLs in your master file.
";

page_tail();
?>
