<?php

require_once("docutil.php");

page_head("Debugging server components");
echo "
<p>
A grab-bag of techniques for debugging BOINC server software:

<h2>Log files</h2>
Most error conditions are reported in the log files.
Make sure you know where these are.
If you're interested in the history of a particular WU or result,
grep for WU#12345 or RESULT#12345 (12345 represents the ID)
in the log files.
The html/ops pages also provide an interface for this.

<h2>Database query tracing</h2>
If you uncomment the symbol SHOW_QUERIES in db/db_base.C,
and recompile everything,
all database queries will be written to stderr
(for daemons, this goes to log files;
for command-line apps it's written to your terminal).
This is verbose but extremely useful for tracking down
database-level problems.

<h2>Scheduler single-stepping</h2>
The scheduler is a CGI program.
It reads from stdin and writes to stdout,
so you can also run it with a command-line debugger like gdb.
Direct a scheduler request file
(which you can copy from a client;
they're saved in files called sched_request_PROJECT.xml)
to stdin, set breakpoints, and start stepping through the code.
<p>
This is useful for figuring out why your project is generating
'no work available' messages.

<h2>MySQL interfaces</h2>
You should become familiar with MySQL tools such as
<ul>
<li> mytop: like 'top' for MySQL
<li> the mysql interpreter ('mysql') and in particular
the 'show processlist;' query.
<li> MySQLadmin: general-purpose web interface to MySQL
</ul>

";

page_tail();
