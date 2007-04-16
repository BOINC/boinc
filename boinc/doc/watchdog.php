<?
require_once("docutil.php");
page_head("Watchdogs");
echo "

<p>
A <b>watchdog</b> is a mechanism for detecting system states
(e.g. full filesystems, database failures, etc.)
that require immediate attention by project staff.
Typically the desired response to such a condition is
to notify a pager, sending a short text description.

<p>
BOINC provides a framework for defining watchdogs:

<ul>
<li>
A set of <b>watchdog scripts</b> are run from cron.
Each script checks for an error condition,
and present, it appends a descriptive line to an error log file.
An example is <b>wd_nresults_changing.php</b>,
which makes sure that the number of results changes.

<li>
The script <b>wd.php</b>, also run from cron,
scans the error log files.
If any has been updated since the last run,
it sends email to a set of recipients,
containing the last line of the file.

</ul>

These files are in the sched/ directory.
";
page_tail();
?>
