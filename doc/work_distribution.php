<?php
require_once("docutil.php");

page_head("Work distribution");

echo "
    A host asks a project for work by including a
".html_text("<work_req_seconds>X</work_req_seconds>")."
element in a scheduler RPC request message.
This asks the scheduler to return enough work to
keep all the host's processors busy for X seconds,
given the host's typical usage
(i.e. the fraction of time it's turned off
or BOINC is suspended, and the other processes that it executes).

<p>
BOINC's work distribution policy addresses the
(sometimes conflicting) goals of keeping hosts as busy as possible,
while minimizing the impact of
<ul>
<li> Hosts that repeatedly return results with error outcomes,
due to a host-specific problem.
<li> Malicious participants who attempt to obtain multiple
results of the same workunit,
in an attempt to obtain unearned credit
or have erroneous results accepted as correct.
</ul>
Work distribution is constrained by a number of rules:
<ul>
<li> A result is sent only if an application version
is available for the host's platform.
<li>
A result is not sent if its disk or memory requirements
are not met by the host.
<li>
A result is not sent if the estimated latency
(based on the host's CPU speed and usage parameters)
exceeds the workunit's latency bound.
<li>
A result is not sent if the project has specified a
<a href=configuration.php>one result per user per workunit</a>
flag, and a result of the same workunit has already been
sent to a host belonging to the same user.
<li>
A result is not sent if the project has specified a 
<a href=configuration.php>daily result quota</a> per host,
and the host has already been sent that many results.
<li>
A project can specify a limit on the
<a href=configuration.php>number of results sent per RPC</a>.
<li>
A result is not sent if
<a href=homogeneous_redundancy.php>homogeneous redundancy</a>
is enabled and another result of the same workunit
has been sent to a different type of host.
<li>
No results are sent of the core client has a different
major version than the scheduling server.
</ul>

In general, the BOINC scheduler responds to a work request
by enumerating unsent results from the database,
filtering them by the above criteria,
sending them to the host,
and continuing until requested duration X is reached.

<p>
For projects that have very large input files,
each of which is used by many workunit,
BOINC offers an alternative work distribution policy
called <a href=sched_locality.php>locality scheduling</a>.
";
page_tail();
?>
