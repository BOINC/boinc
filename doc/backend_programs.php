<?php
require_once("docutil.php");
page_head("Daemon programs");
echo "

<p>
A BOINC project includes of a set of daemons
(programs that run all the time).
Each program should be listed as a daemon in the
<a href=configuration.php>config.xml</a> file.
They all have the command-line option:
<pre>
-d N
</pre>
Sets the verbosity level.
1 = critical messages only,
2 = normal messages,
3 = detailed debugging info.

<h3>Work generator</h3>
<p>
There is one work generator per application.
It creates workunits and the corresponding input files.
It is application-specific, and uses
<a href=tools_work.php>BOINC library functions</a>
for registering the workunits in the database.
<p>
During testing, you can create a single workunit using
<a href=tools_work.php>create_work</a>,
then use the daemon program
<a href=busy_work.php>make_work</a>
to copy this workunit as needed to maintain a given supply of work.

<h3>Transitioner</h3>
<p>
This program is supplied by BOINC and is application independent.
It handles state transitions of workunits and results.
It generates initial results for workunits,
and generates more results when timeouts or errors occur.
It has the command-line option:
<pre>
-mod n i
</pre>
If present, this instance of the transitioner will handle
only workunits for which mod(id, n) = i.
This lets you run arbitrarily many instances of the transitioner.

<h3>Validator</h3>
<p>
There is one validator per application.
It compares redundant results and selects a <b>canonical result</b>
representing the correct output,
and a <b>canonical credit</b> granted to users and hosts
that return the correct output.

<h3>Assimilator</h3>
There is one assimilator per application.
It handles workunits that are 'completed':
that is, that have a canonical result or for which
an error condition has occurred.
Handling a successfully completed result might involve
record results in a database and perhaps generating more work.

<h3>File deletion</h3>
The application-independent program
<a href=file_deleter.php>file_deleter</a>
deletes input and output files when they are no longer needed.

<h3>Database purging</h3>
The application-independent program
<a href=db_purge.php>db_purge</a>
removes work-related database entries when they are no longer needed.
This keeps your database at a constant size even
when your project runs for a long time.
";
page_tail();
?>
