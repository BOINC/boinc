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
<li> At any given point, a computer should have enough work
so that NCPUS processors will be busy for at least
min_queue days (min_queue is a user preference).
<li> Project resource shares should be honored over the long term.
<li> Variety: if a computer is attached to multiple projects,
execution should rotate among projects on a frequent basis.
</ul>

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
the CPU scheduling policy optimizes the likelihood of meeting deadlines,
at the expense of variety.
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
R is <b>runnable</b> if
<ul>
<li> Neither R nor R.project is suspended, and
<li> R's files have been downloaded, and
<li> R hasn't finished computing
</ul>

<h3>Project states</h3>
P is <b>runnable</b> if
<ul>
<li> P has at least one runnable result
(this implies that P is not suspended).
</ul>

P is <b>downloading</b> if
<ul>
<li> P is not suspended, and
<li> P has at least one result whose files are being downloaded
and none of the downloads is deferred.
</ul>

P is <b>fetchable</b>
(i.e. the work-fetch policy allows work to be fetched from it) if
<ul>
<li> P is not suspended, and
<li> P is not deferred (i.e. its minimum RPC time is in the past), and
<li> P's no-new-work flag is not set, and
<li> P is not overworked (see definition below), and
<li> a fetch of P's master file is not pending
</ul>

P is <b>latency-limited</b> if
<ul>
<li> The client's last scheduler RPC to P returned
a 'no work because of deadlines' flag, and
<li> the RPC reply's delay request has not yet elapsed.
</ul>
This means that P has work available,
but didn't send any because the work's deadlines couldn't be met
given the existing work queue.
<p>
P is <b>potentially runnable</b> if
<ul>
<li> P is either runnable, downloading, fetchable, overworked,
or latency-limited.
</ul>
This means that, to the best of the client's knowledge,
it could do work for P if it wanted to.

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
<li> P.debt is normalized so that the mean or minimum is zero.
</ul>


<b>Short-term debt</b> is used by the CPU scheduler.
It is adjusted over the set of runnable projects.
It is normalized so that minimum short-term debt is zero,
and maximum short-term debt is no greater than 86,400 (i.e. one day).

<p>
<b>Long-term debt</b> is used by the work-fetch policy.
It is defined for all projects,
and adjusted over the set of potentially runnable projects.
It is normalized so that average long-term debt,
over all project, is zero.

<h2>CPU scheduling policy</h2>

<p>
The CPU scheduler uses an earliest-deadline-first (EDF) policy
for results that are in danger of missing their deadline,
and weighted round-robin among other projects if additional CPUs exist.
This allows the client to meet deadlines that would otherwise be missed,
while honoring resource shares over the long term.
<p>
The scheduler starts by doing a simulation of round-robin scheduling
applied to the current work queue.
This produces the following outputs:
<ul>
<li> deadline_missed(R): whether result R misses its deadline.
<li> deadlines_missed(P):
the number of results R of P for which deadline_missed(R).
<li> total_work_before_minq:
the wall CPU time used in the next min_queue seconds
(this is used by the work-fetch policy, see below).
<li> work_before_minq(P):
the wall CPU time used by project P in the next min_queue seconds
(this is used by the work-fetch policy, see below).
</ul>
The scheduling policy is:
<ol>
<li> Let P be the project with the earliest-deadline runnable result
among projects with deadlines_missed(P)>0.
Let R be P's earliest-deadline runnable result not scheduled yet.
Tiebreaker: least index in result array.
<li> If such an R exists, schedule R and decrement deadlines_missed(P).
<li> If there are more CPUs, and projects with deadlines_missed(P)>0, go to 1.
<li> If all CPUs are scheduled, stop.
<li> Set the 'anticipated debt' of each project to its short-term debt
<li> Find the project P with the greatest anticipated debt,
select one of P's runnable results
(picking one that is already running, if possible,
else the result with earliest deadline)
and schedule that result.
<li> Decrement P's anticipated debt by the 'expected payoff'
(the scheduling period divided by NCPUS).
<li> Repeat steps 6 and 7 for additional CPUs
</ol>

<p>
The CPU scheduler runs when a result is completed,
when the end of the user-specified scheduling period is reached,
when new results become runnable,
or when the user performs a UI interaction
(e.g. suspending or resuming a project or result).

<p>
The CPU scheduler decides what result should run,
but it doesn't enforce this decision
(by preempting, resuming and starting applications).
This enforcement is done by a separate function,
which runs periodically, and is also called by
the CPU scheduler at its conclusion.
The following rules apply to application preemption:
<ul>
<li> If the 'leave in memory' preference is not set,
an application scheduled for preemption is allowed to run for
up to sched_interval/2 additional seconds, or until it checkpoints.
<li>
The above does not apply for application being preempted
to run a result R for which deadline_missed(R).
<li> If an application has never checkpointed,
it is always left in memory on preemption.
</ul>


<h2>Work-fetch policy</h2>

<p>
When a result runs in EDF mode,
its project may get more than its share of CPU time.
The work-fetch policy is responsible for
ensuring that this doesn't happen repeatedly.
It does this by suppressing work fetch for the project.
<p>
A project P is <b>overworked</b> if
<ul>
<li> P.long_term_debt < -sched_period
</ul>
<p>
This condition occurs if P's results run in EDF mode
(and in extreme cases, when a project with large negative LTD is detached).
The work-fetch policy avoids getting work from overworked projects.
This prevents a situation where a project with short deadlines
monopolizes the CPU.

<p>
The work-fetch policy uses the functions
<pre>
frs(project P)
</pre>
<blockquote>
P's fractional resource share among fetchable projects.
</blockquote>

<pre>
work_to_fill_buf(P)
</pre>
The amount of work needed to keep P busy for the next min_queue seconds,
namely:
<pre>
    y = min_queue*ncpus - work_before_minq(P)
    if (y <= 0) return 0
    return y/frs(P)
</pre>
<p>
The work-fetch policy function is called every few minutes
(or as needed) by the scheduler RPC polling function.
It sets the variable <b>work_request_size(P)</b> for each project P,
which is the number of seconds of work to request
if we do a scheduler RPC to P.
This is computed as follows:
<pre>
for each project P
    if P is suspended, deferred, overworked, or no-new-work
        P.work_request_size = 0
    else
        P.work_request_size = work_to_fill_buf(P)

if min_queue*ncpus > total_work_before_minq
    if P.work_request_size==0 for all P
        for each project P
            if P is suspended, deferred, overworked, or no-new-work
                continue
            P.work_request_size = 1

    if P.work_request_size==0 for all P
        for each project P
            if P is suspended, deferred, or no-new-work
                continue
            P.work_request_size = 1

    if P.work_request_size>0 for some P
        Normalize P.work_request_size so that they
        sum to min_queue*ncpus - total_work_before_minq
        and are proportional to P.resource_share
</pre>

<p>
The scheduler RPC mechanism may select a project to contact
because of a user request, an outstanding trickle-up message,
or a result that is overdue for reporting.
If it does so, it will also request work from that project.
Otherwise, the RPC mechanism chooses the project P for which
<pre>
P.work_request_size>0 and
P.long_term_debt + work_to_fill_buf(P) is greatest
</pre>
and gets work from that project.
<hr>
<h2>Scheduler work-send policy</h2>
<p>
NOTE: the following has not been implemented,
and is independent of the above policies.
<p>
The scheduler should avoid sending results whose
deadlines are likely to be missed,
or which are likely to cause existing results to miss their deadlines.
This will be accomplished as follows:
<ul>
<li>
Scheduler requests includes connection period,
list of queued result (with estimated time remaining and deadline)
and project resource fractions.
<li>
The scheduler won't send results whose deadlines are less than
now + min_queue.
<li>
The scheduler does an EDF simulation of the initial workload
to determine by how much each result misses its deadline.
For each result R being considered for sending,
the scheduler does an EDF simulation.
If R meets its deadline
(optional if the project does not need strict adherence),
and no result misses its deadline by more than it did previously, R is sent.

<li>
If the scheduler has work but doesn't send any because of deadline misses,
it returns a 'no work because of deadlines' flag.
If the last RPC to a project returned this flag,
it is marked as latency-limited and accumulates LTD.
</ul>
<hr>
<h2>Describing scenarios</h2>
<p>
We encourage the use of the following notation for
describing scheduling scenarios
(times are given in hours):
<p>
P(C, D, R)
<p>
This describes a project with
<ul>
<li> C = CPU time per task
<li> D = delay bound
<li> R = fractional resource share
</ul>

A scenario is described by a list of project,
plus the following optional parameters:
<ul>
<li> NCPUS: number of CPUS (default 1)
<li> min_queue
</ul>

Hence a typical scenario description is:
<pre>
(P1(1000, 2000, .5), P2(1, 10, .5), NCPUS=4)
</pre>
";
page_tail();
?>
