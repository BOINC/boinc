<?php

require_once("docutil.php");

page_head("Multiple server hosts");

echo"
When you initially create a BOINC project using
<a href=make_project.php>make_project</a>,
everything runs on a single host:
web server, scheduling server, daemons, tasks, database server,
file upload handler.

<p>
You can increase the capacity of your server by spreading
these tasks across different hosts.
The following rules apply:

<ul>
<li> The hosts should share a common network file system,
and path of the project directory, on any host,
should map to the same place.
(Exception: the database server can run anywhere).

<li> The <a href=groups.php>project admin account</a>
should exist on all hosts,
and that user should be able to use 'ssh' to run
commands on any other host without typing a password.

<li> One host is designated as the project's <b>main host</b>
in config.xml.
<b>The 'start', 'stop', and 'status' scripts should normally
be run on the main host</b>
(if you run them on a different host X,
they'll affect only daemons and tasks on host X).

<li> The project admin account on all hosts must be
able to access the project's MySQL database.
(Exception: data servers and file upload handlers
don't need DB access).
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

";

page_tail();
?>
