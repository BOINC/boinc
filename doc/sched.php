<?php
require_once("docutil.php");
page_head("Client scheduling");
echo "

This document describes two related parts of the BOINC core client
(version 4.36 and later):
<p>
<b>CPU scheduling policy</b>:
Of the set of results that are runnable (see below),
which ones to execute?
(On a machine with N CPUs, BOINC will try to execute N results at once).

<p>
<b>Work-fetch policy</b>:
When should the core client ask a project for more work,
which project should it ask,
and how much work should it ask for?

<p>
The goals of the CPU scheduler and work-fetch policies are
(in descending priority):
<ul>
<li> Results should be completed and reported by their deadline
(results reported after their deadline
may not have any value to the project and may not be granted credit).
<li> Project resource shares should be honored over the long term;
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


<h2>The CPU scheduling policy</h2>
<p>
The CPU scheduler has two modes, <b>normal</b> and <b>panic</b>.
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
    (the total wall CPU in the last period divided by #CPUs).
<li> Repeat steps 2 and 3 for additional CPUs
</ol>
Over the long term, this results in a round-robin policy,
weighted by resource shares.

<p>
In panic mode, the CPU scheduler
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
runnable results will fall below #CPUs.
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
    X &lt; than min_queue 
<li>
<b>OK</b>:
    X > min_queue, work_fetch_OK is true
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
will fall below #CPUs*R(P)
<ul>
<li>
<b>NEED_IMMEDIATELY</b>:
    no results of P are runnable soon.
<li>
<b>NEED</b>:
    X(P) < min_queue * R(P)
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
Sort the work units by deadline, earliest first.
If at any point in this list, the sum of the remaining 
processing time is greater than 0.8 * up_frac * time to deadline,
the CPU queue is overloaded.
This triggers both no work requests and the CPU scheduler
into earliest deadline first.

<p>
Sum the fraction that the remaining processing time is of the time
to deadline for each work unit.
If this is greater than 0.8 * up_frac, the CPU queue is fully loaded.
This triggers no work fetch.



";
page_tail();
?>
