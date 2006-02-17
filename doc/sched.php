<?php
require_once("docutil.php");
page_head("Client scheduling policies");
echo "

This document describes two related parts of the BOINC core client:
<dl>
<dt><b>CPU scheduling policy</b>
<dd>
Of the results that are runnable, which ones to execute?
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
(because results reported after their deadline
may not have any value to the project and may not be granted credit).
<li> NCPUS processors should be kept busy.
<li> At any given point, enough work should be kept on hand
so that NCPUS processors will be busy for at least
min_queue days (min_queue is a user preference).
<li> Project resource shares should be honored over the long term.
<li> Variety: if a computer is attached to multiple projects,
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
The actual CPU time may be less than this,
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
each is defined for a set S of projects.
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
It is defined for all projects,
and adjusted over the set of potentially runnable projects.
It is normalized so that average long-term debt is zero.

<h2>The CPU scheduling policy</h2>

<p>
The CPU scheduler uses an earliest-deadline-first (EDF) policy
for results that are in danger of missing their deadline,
and round-robin among other projects if additional CPUs exist.
This allows the client to meet deadlines that would otherwise be missed,
while honoring resource shares over the long term.
The scheduler uses the following data, which are obtained
by a simulation of round-robin scheduling
applied to the current work queue:
<ul>
<li> deadline_missed(R): whether result R would miss
its deadline with round-robin scheduling.
<li> deadlines_missed(P):
the number of results of P whose deadlines would
be missed with round-robin scheduling.
</ul>
The scheduling policy is:
<ol>
<li> Find the project P for which deadlines_missed(P)>0,
and whose earliest deadline is earliest.
<li> Schedule deadline_missed(P)
results of P in order of increasing deadline,
with preference to those already running.
<li> If there are more CPUs, and other projects for which
deadlines_missed(P)>0, go to 1.
<li> If all CPUs are scheduled, stop;
otherwise continue to the next step, considering
only projects with deadlines_missed==0.
<li> Set the 'anticipated debt' of each project to its short-term debt
<li> Find the project P with the greatest anticipated debt,
select one of P's runnable results
(picking one that is already running, if possible)
and schedule that result.
<li> Decrement P's anticipated debt by the 'expected payoff'
    (the total wall CPU in the last period divided by NCPUS).
<li> Repeat steps 6 and 7 for additional CPUs
</ol>

<p>
The CPU scheduler runs when a result is completed,
when the end of the user-specified scheduling period is reached,
when new results become runnable,
or when the user performs a UI interaction
(e.g. suspending or resuming a project or result).

<h2>Work-fetch policy</h2>

<p>
The work-fetch policy uses the functions
<pre>
prrs(project P)
</pre>
<blockquote>
P's fractional resource share among potentially runnable projects.
</blockquote>

<pre>
min_results(project P)
</pre>
<blockquote>
The minimum number of runnable results needed to
maintain P's resource share on this machine: namely,
<br>
ceil(ncpus*prrs(P))
</blockquote>
<pre>
time_until_work_done(project P)
</pre>
<blockquote>
The estimated wall time until the number of
uncompleted results for this project will reach min_results(P)-1,
assuming round-robin scheduling among
the current potentially runnable projects.
</blockquote>
<p>
The work-fetch policy function is called every 5 seconds
(or as needed) by the scheduler RPC polling function.
</pre>
It sets the following variable for each project P:
<p>
    <b>work_request_size(P)</b>:
    the number of seconds of work to request if we do a scheduler RPC to P.
    This is
    <ul>
    <li>
    0 if P is suspended, deferred, or no-new-work
    <li>
    0 if time_until_work_done(P) > min_queue
    <li>
    0 if CPU scheduler scheduled all CPUs by EDF
    <li>
    0 if P.long_term_debt < -(CPU sched interval)
        (project has been using more than its share of CPU;
         e.g. repeatedly doing short-deadline results).
    <li>
    otherwise:
    (min_queue*ncpus*prrs(P)) - (estimated wall time of queued work)
    </ul>

<p>
The scheduler RPC mechanism may select a project to contact
because of a user request, an outstanding trickle-up message,
or a result that is overdue for reporting.
If it does so, it will also request work from that project.
Otherwise, the RPC mechanism chooses the project P for which
<pre>
P.work_request_size>0 and
P.long_term_debt - time_until_work_done(P) is greatest
</pre>
and gets work from that project.
<hr>
Scheduler request must include:
whether sporadic network connection (period);
whether in EDF mode.
Resource fractions?
<p>
Scheduler reply should include (it no work sent)
if reason is because we're overloaded.
<p>
If last response was 'no work', don't accumulate LTD.

";
page_tail();
?>
