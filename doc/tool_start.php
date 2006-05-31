<?php
require_once("docutil.php");
page_head("Project control");
echo "
<h2>Trigger files</h2>
<p>
The following files (in the project root directory)
can be used to turn off various parts of a project.
<pre>
   stop_sched
</pre>
Have the scheduler return 'project down' messages.
<pre>
   stop_daemons
</pre>
Tell all daemon process to exit.

<pre>
   stop_web
</pre>
Have the web site return 'project down' messages
for all functions that require database access.

<pre>
   stop_upload
</pre>
Have the file upload handler return transient error messages to clients
(they'll back off and retry later).

<p>
The presence of a file triggers the function.
For example, to turn off data-driven web pages, type
<pre>
   touch stop_web
</pre>
and to them back on, type
<pre>
   rm stop_web
</pre>

<p>
If all three files are present, no database access will occur.
You should do this during most maintenance functions
(such as upgrading software).

<h2>Project control scripts</h2>

The following Python scripts control a project:
<pre>
   bin/start
</pre>
Start the project: start all daemons,
and remove the stop_sched and stop_daemon files.
<pre>
   bin/stop
</pre>
Stop the project (create the stop_sched and stop_daemon files)
<pre>
   bin/start --cron
</pre>
If the project is started, perform all periodic tasks that are past due,
and start any daemons that aren't running.
Otherwise do nothing.
<pre>
   bin/status
</pre>
Show whether the project is stopped.
Show the status of all daemons.
Show the status of all periodic tasks
(e.g., when they were last executed).

";
page_tail();
?>

