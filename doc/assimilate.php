<?php
require_once("docutil.php");
page_head("Result assimilation");
echo "
Projects must create one assimilator program per application.
This is done by linking the program <b>sched/assimilate.C</b>
with an application-specific function of the form
<pre>
int assimilate_handler(
    WORKUNIT& wu, vector&lt;RESULT>& results, RESULT& canononical_result
);
</pre>

This is called when either
<ul>
<li> The workunit has a nonzero
<a href=work.php>error mask</a>
(indicating, for example, too many error results).
In this case the handler might write a message to a log
or send an email to the application developer.
<li> The workunit has a canonical result.
In this case wu.canonical_resultid will be nonzero,
canonical_result will contain the canonical result.
Your handler might, for example, parse the canonical result's
output file and write its contents to a separate database.
</ul>
In both cases the 'results' vector will be populated with
all the workunit's results (including unsuccessful and unsent ones).
All files (both input and output) will generally be on disk.
<p>
It's possible that both conditions might hold.
<p>
If assimilate_handler() returns zero,
the workunit record will be marked as assimilated.
If assimilate_handler() returns nonzero,
the assimilator will log an error message and exit.
Typically you should return nonzero for any recoverable error,
to stop the assimilator from running.
This way the system administrator can fix the problem before any completed
or erroneous workunits are mis-handled by BOINC.
<p>
You can use BOINC's
<a href=backend_util.php>back-end utility functions</a>
to get file pathnames and open files.

";
page_tail();
?>
