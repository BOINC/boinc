<?
require_once("docutil.php");
page_head("Back end daemons");
echo "

<p>
A project <b>back end</b> consists of a set of daemons
(programs that run all the time),
some of which have application-specific parts.
Each program should be listed as a daemon in the
<a href=configuration.php>config.xml</a> file.

<h3>Work generation</h3>
<p>
There is one work generator per application.
It creates workunits and the corresponding input files.
It is entirely application-specific, and uses
<a href=tools_work.php>BOINC-supplied interfaces</a>
for registering the workunits in the database.

<h3>Transitioning</h3>
<p>
This program (which is supplied by BOINC and is application independent)
handles various state transitions of workunits and results,
such as timeouts.
It generates initial results for workunits,
and generates more results when timeouts or errors occur.

<h3>Validation</h3>
<p>
There is one validator per application.
It compares redundant results and selects a <b>canonical result</b>
representing the correct output,
and a <b>canonical credit</b> granted to users and hosts
that return the correct output.

<h3>Assimilation</h3>
There is one assimilator per application.
It handles workunits that are 'completed':
that is, that have a canonical result or for which
an error condition has occurred.
Handling a successfully completed result might involve
record results in a database and perhaps generating more work.

<h3>File deletion</h3>
This application-independent program
deletes input and output files
when they are no longer needed.
";
page_tail();
?>
