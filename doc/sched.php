<?php
require_once("docutil.php");
page_head("Client scheduling");
echo "

This document describes two related parts of the BOINC core client
(version 4.36 and later):
<dl>
<dt><b>CPU scheduling policy</b>
<dd>
Of the set of results that are runnable (see below),
which ones to execute?
BOINC will generally execute NCPUS results at once,
where NCPUS is the minimum of the physical number of CPUs
(counting hyperthreading) and the user's 'max_cpus' general preference.

<dt><b>Work-fetch policy</b>
<dd>
When should the core client ask a project for more work,
which project should it ask,
and how much work should it ask for?
</dl>

<p>
The goals of the CPU scheduler and work-fetch policies are
(in descending priority):
<ul>
<li> Results should be completed and reported by their deadline
(results reported after their deadline
may not have any value to the project and may not be granted credit).
<li> NCPUS processors should be kept busy.
<li> At any given point, enough work should be kept on hand
so that NCPUS processors will be busy for at least
min_queue days (min_queue is a user preference).
<li> Project resource shares should be honored over the long term.
<li> If a computer is attached to multiple projects,
    execution should rotate among projects on a frequent basis.
</ul>
The policies are designed to accommodate all scenarios,
including those with computers that are slow or are attached
to a large number of projects.

<p>
In previous versions of BOINC,
the core client attempted to maintain at least one result
for each attached project,
and would do weighted round-robin CPU scheduling among all projects.
In some scenarios (any combination of slow computer,
lots of projects, and tight deadlines) a computer could
miss the deadlines of all its results.
The new policies solve this problem as follows:
<ul>
<li>
Work fetch is limited to ensure that deadlines can be met.
A computer attached to 10 projects might
have work for only a few (perhaps only one) at a given time.
<li>
If deadlines are threatened,
the CPU scheduling policy switches to a mode
(earliest deadline first) that optimizes the likelihood
of meeting deadlines, at the expense of variety.
</ul>



<h2>Concepts and terms</h2>

<h3>Wall CPU time</h3>
A result's <b>wall CPU time</b> is the amount of wall-clock time
its process has been runnable at the OS level.
The actual CPU time may be much less than this,
e.g. if the process does a lot of paging,
or if other (non-BOINC) processing jobs run at the same time.
<p>
BOINC uses wall CPU time as the measure of how much resource
has been given to each project.
Why not use actual CPU time instead?
<ul>
<li> Wall CPU time is more fair in the case of paging apps.
<li> The measurement of actual CPU time depends on apps to
report it correctly.
Sometimes apps have bugs that cause them to always report zero.
This screws up the scheduler.
</ul>



<h3>Result states</h3>
A result is <b>runnable</b> if
<ul>
<li> Neither it nor its project is suspended, and
<li> its files have been downloaded, and
<li> it hasn't finished computing
</ul>
A result is <b>runnable soon</b> if
<ul>
<li> Neither it nor its project is suspended, and
<li> it hasn't finished computing
</ul>


<h3>Project states</h3>
A project is <b>runnable</b> if
<ul>
<li> It's not suspended, and
<li> it has at least one runnable result
</ul>

A project is <b>downloading</b> if
<ul>
<li> It's not suspended, and
<li> it has at least one result whose files are being downloaded
</ul>

A project is <b>contactable</b> if
<ul>
<li> It's not suspended, and
<li> its master file has already been fetched, and
<li> it's not deferred (i.e. its minimum RPC time is in the past), and
<li> it's no-new-work flag is not set
</ul>

A project is <b>potentially runnable</b> if
<ul>
<li> It's either runnable, downloading, or contactable.
</ul>

<h3>Debt</h3>
Intuitively, a project's 'debt' is how much work is owed to it,
relative to other projects.
BOINC uses two types of debt;
each is defined related to a set S of projects.
In each case, the debt is recalculated periodically as follows:
<ul>
<li> A = the wall CPU time used by projects in S during this period
<li> R = sum of resource shares of projects in S
<li> For each project P in S:
   <ul>
   <li> F = P.resource_share / R (i.e., P's fractional resource share)
   <li> W = A*F (i.e., how much wall CPU time P should have gotten)
   <li> P.debt += W - P.wall_cpu_time (i.e. what P should have gotten
           minus what it got).
    </ul>
<li> P.debt is normalized (e.g. so that the mean or minimum is zero).
</ul>


<b>Short-term debt</b> is used by the CPU scheduler.
It is adjusted over the set of runnable projects.
It is normalized so that minimum short-term debt is zero,
and maximum short-term debt is no greater than 86400 (i.e. one day).

<p>
<b>Long-term debt</b> is used by the work-fetch policy.
It is adjusted over the set of potentially runnable projects.
It is normalized so that average long-term debt is zero.

<h3>Required time fraction</h3>

A result's <b>required time fraction</b> (RTF) is its
estimated remaining CPU time divided by the time to its deadline.
SHOULD WE ALSO TAKE INTO ACCOUNT FACTORS BESIDES CPU TIME?

<p>
A result's <b>cumulative required time fraction</b> (CRTF)
of a result R is the estimated remaining CPU time
of R and all results with earlier deadlines,
divided by the time until R's deadline.

<p>
Example:
<table cellpadding=6 border=1>
<tr>
    <th>Result</th>
    <th>Remaining CPU time</th>
    <th>Time until deadline</th>
    <th>RTF</th>
    <th>CRTF</th>
</tr>
<tr>
    <td>A</td>
    <td>5 hours</td>
    <td>50 hours</td>
    <td>0.1</td>
    <td>0.1</td>
</tr>
<tr>
    <td>B</td>
    <td>76 hours</td>
    <td>100 hours</td>
    <td>0.76</td>
    <td>0.81</td>
</tr>
<tr>
    <td>C</td>
    <td>500 hours</td>
    <td>5000 hours</td>
    <td>0.1</td>
    <td>0.12</td>
</tr>
</table>

<h2>The CPU scheduling policy</h2>
<p>
The CPU scheduler has two modes, <b>normal</b> and
<b>Earliest Deadline First (EDF)</b>.
In normal mode, the CPU scheduler runs the project(s)
with the greatest short-term debt.
Specifically:
<ol>
<li> Set the 'anticipated debt' of each project to its short-term debt
<li> Find the project P with the greatest anticipated debt,
    select one of P's runnable results
    (picking one that is already running, if possible)
    and schedule that result.
<li> Decrement P's anticipated debt by the 'expected payoff'
    (the total wall CPU in the last period divided by NCPUS).
<li> Repeat steps 2 and 3 for additional CPUs
</ol>
Over the long term, this results in a round-robin policy,
weighted by resource shares.

<p>
In EDF mode, the CPU scheduler
schedules the runnable results with the earliest deadlines.
This allows the client to meet deadlines that would otherwise be missed.


<p>
The CPU scheduler runs when a result is completed,
when the end of the user-specified scheduling period is reached,
when new results become runnable,
or when the user performs a UI interaction
(e.g. suspending or resuming a project or result).


<h2>The work-fetch policy</h2>

<p>
X is the estimated wall time by which the number of
runnable results will fall below NCPUS.
<p>
min_queue is the user's network-connection period general preference.
<p>
work_fetch_OK is a flag set by the mode selection algorithm (see below).
<p>
The work-fetch policy maintains an 'overall urgency':
<ul>
<li>
<b>NEED_IMMEDIATELY</b>:
    there is at least one idle CPU
<li>
<b>NEED</b>:
    X &lt; min_queue 
<li>
<b>OK</b>:
    X &gt; min_queue, work_fetch_OK is true
<li>
<b>DONT_NEED</b>:
    work_fetch_OK is false
</ul>

<p>
In addition, the work-fetch policy maintains a per-project work-fetch mode:
<p>
R(P) = fractional resource share of P
<p>
X(P) = estimated wall time when number of runnable results for P
will fall below NCPUS*R(P)
<ul>
<li>
<b>NEED_IMMEDIATELY</b>:
    no results of P are runnable soon.
<li>
<b>NEED</b>:
    X(P) &lt; min_queue * R(P)
<li>
<b>OK</b>:
    X(P) > min_queue * R(P),
    and P is not suspended or deferred or no-new-work
<li>
<b>DONT_NEED</b>:
    P is suspended or deferred or no-new-work
</ul>

<p>

<h2>Mode selection</h2>
<p>
Work_fetch_OK is set to false if either
<ul>
<li> The sum of all RTFs is > 0.8
<li> The CRTF of any result is > 0.8
</ul>

EDF mode is used if either
<ul>
<li> The CRTF of any result is > 0.8
<li> The deadline of any result is earlier than one day from now
<li> The deadline of any result is less than
2 * min_queue from now.
</ul>



";
page_tail();
?>
