<?php
require_once("docutil.php");
page_head("Project control");
echo "
<h2>Starting and stopping a project</h2>

While a project is <b>stopped</b>, its daemons are stopped,
the scheduler returns 'project down' messages,
and the web site displays a 'Project down' page.
No database accesses occur while a project is stopped.
A project should be stopped during most maintenance functions
(such as upgrading software).
<p>
The following scripts (in bin/) control a project:
<p>
<b>start</b>
<p>
Start the project.
<p>
<b>stop</b>
<p>
Stop the project.
<p>
<b>cron (invoked as 'start --cron')</b>
<p>
If the project is started, perform all periodic tasks
that are past due, and start any daemons that aren't running.
Otherwise do nothing.
<p>
<b>status</b>
<p>
Show whether the project is stopped.
Show the status of all daemons.
Show the status of all periodic tasks
(e.g., when they were last executed).

<h2>Turning off the scheduler</h2>

If you want to turn off the scheduler but leave everything else running,
create a file
<pre>
stop_sched
</pre>
in the project directory.
Remove it to reactivate the scheduler.
";
page_tail();
?>

