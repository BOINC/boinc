<?php
require_once("docutil.php");
page_head("Project control");
echo "
<h2>Project control scripts</h2>

The following Python scripts control a project:
";
list_start();
list_item("bin/start",
    "Start the project: start all daemons,
    and remove the stop_sched and stop_daemon files (see below)."
);
list_item("bin/stop", 
    "Stop the project (create the stop_sched and stop_daemon files)"
);
list_item("bin/start --cron",
    "If the project is started, perform all periodic tasks that are past due,
    and start any daemons that aren't running.
    Otherwise do nothing."
);
list_item("bin/status",
    "Show whether the project is stopped.
    Show the status of all daemons.
    Show the status of all periodic tasks
    (e.g., when they were last executed)."
);
list_end();
echo "
<h2>Trigger files</h2>
<p>
The following files (in the project root directory)
can be used to turn off various parts of a project.
";
list_start();
list_item("stop_sched",
    "Have the scheduler return 'project down' messages."
);
list_item("stop_daemons",
    "Tell all daemon process to exit."
);
list_item("stop_web",
    "Have the web site return 'project down' messages
    for all functions that require database access."
);
list_item("stop_upload",
    "Have the file upload handler return transient error messages to clients
    (they'll back off and retry later)."
);
list_end();
echo "
<p>
The presence of a file triggers the function.
For example, to turn off data-driven web pages, type
<pre>
   touch stop_web
</pre>
and to turn them back on, type
<pre>
   rm stop_web
</pre>

<p>
If the first three files are all present, no database access will occur.
You should do this during most maintenance functions
(such as upgrading software).
";

page_tail();
?>
