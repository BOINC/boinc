<?php

require_once("docutil.php");

page_head("Daemons");

echo "
<b>Daemons</b> are server programs that normally run continuously.
Your project's daemons are described in its
<a href=configuration.php>config.xml</a> file,
with elements of the form:

";
echo html_text("
<daemon>
  <cmd> feeder -d 3 </cmd>
  [ <host>host.domain.name</host> ]
  [ <disabled> 0|1 </disabled> ]
  [ <output>filename</output> ]
  [ <pid_file>filename</pid_file> ]
</daemon>
<daemon>
...
</daemon>");
list_start();
list_item("cmd",
    "The command used to start the daemon.
    This command will be run in the project's bin/ directory."
);
list_item("host",
    "Specifies where the daemon should run.
    The default is the project's main host,
    as specified in config.xml."
);
list_item("disabled",
    "If nonzero, ignore this entry"
);
list_item("output",
    "Name of output file.
    Defaults to the program name followed by '.log'.
    If you're running multiple instances of a daemon on one host,
    you must specify this."
);
list_item("pid_file",
    "Name of file used to store the process ID.
    Defaults to the program name followed by '.pid'.
    If you're running multiple instances of a daemon on one host,
    you must specify this."
);
list_end();
echo "

<p>
Daemons are started when you run the
<a href=tool_start.php>bin/start</a> script,
and killed (by a SIGHUP signal) when you run
<a href=tool_start.php>bin/stop</a>.
The process ID is recorded in the &lt;pid_dir> directory
By convention, daemon X running on host Y
writes it log output to log_Y/X.log.

<p>
Typically, this mechanism is used to run
<a href=backend_programs.php>work handling daemons</a>.
Projects that use trickle-up messages will also need
to have a <a href=trickle.php>trickle-up handler</a>.
";
page_tail();
?>
