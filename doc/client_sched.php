<?php
require_once("docutil.php");
page_head("Client scheduling policies");
echo "

This document describes two interrelated scheduling policies
in the BOINC client:

<ul>
<li> <b>CPU scheduling policy</b>: what result to run when.
<li> <b>Work fetch policy</b>: when to contact scheduling servers,
which projects to contact, and how much work to task for.
</ul>

<h2>CPU scheduling</h2>

<p>The CPU scheduling policy aims to achieve the following goals
(in decreasing priority):</p>

<ol>

<li>
<b>Maximize CPU utilization</b>.

<li>
<b>Enforce resource shares.</b>
The ratio of CPU time allocated to projects that have work,
in a typical period of a day or two,
should be approximately the same as the ratio of
the user-specified resource shares.
If a process has no work for some period,
it does no accumulate a 'debt' of work.

<li>
<b>Satisfy result deadlines if possible.</b>

<li>
<b>Reschedule CPUs periodically.</b>
This goal stems from the large differences in duration of
results from different projects.
Participant in multiple projects
will expect to see their computers do work on each of these projects in a
reasonable time period.

<li>
<b>Minimize mean time to completion.</b>
For example, it's better to have one result from
a project complete in time T than to have two results
simultaneously complete in time 2T.

</ol>

<p>
A result is 'active' if there is a slot directory for it.
There can be more active results than CPUs.


<h3>Debt</h3>

<p>
The notion of 'debt' is used to respect the resource share allocation
for each project.
The debt to a project represents the amount of work
(in CPU time) we owe it.
Debt is decreased when CPU time is devoted to a project.
We increase the debt to a project according to the
total amount of work done in a time period scaled by the project's
resource share.

<p>
For example, consider a system participating in two projects, A and B,
with resource shares 75% and 25%, respectively.
Suppose in some time period, the system devotes 25 minutes of CPU time to project A
and 15 minutes of CPU time to project B.
We decrease the debt to A by 25 minutes and increase it by 30 minutes (75% of 25 + 15).
So the debt increases overall.
This makes sense because we expected to devote a
larger percentage of the system resources to project A than it
actually got.

<p>
The choice of projects for which to start result computations
can simply follow the debt ordering of the projects.
The algorithm computes the 'anticipated debt' to a project
(the debt we expect to owe after the time period expires)
as it chooses result computations to run.

<h3>A sketch of the CPU scheduling algorithm</h3>

<p>
This algorithm is run:
<ul>
<li> Whenever a CPU is free
<li> Whenever a new result arrives (via scheduler RPC)
<li> Whenever it hasn't run for MV seconds
</ul>

<p>
We will attempt to minimize the number of active result
computations for a project by dynamically choosing results to compute
from a global pool.
When we allocate CPU time to project,
we will choose already running tasks first,
then preempted tasks, and only choose to start a new result
computation in the last resort.
This will not guarantee the above
property, but we hope it will be close to achieving it.

<ol>

<li>Decrease debts to projects according to the amount of work done for
the projects in the last period.

<li>Increase debts to projects according to the projects' resource shares.

<li>Let the anticipated debt for each project be initialized to
its current debt.

<li>Repeat until we decide on a result to compute for each processor:

<ol>

<li>Choose the project that has the largest anticipated debt and a
ready-to-compute result.

<li>Decrease the anticipated debt of the project by the expected amount of CPU time.

</ol>

<li>Preempt current result computations, and start new ones.

</ol>

<h3>Pseudocode</h3>

<pre>
data structures:
ACTIVE_TASK:
    double period_start_cpu_time
    double current_cpu_time
    scheduler_state:
        PREEMPTED
        RUNNING
    next_scheduler_state    // temp
PROJECT:
    double work_done_this_period    // temp
    double debt
    double anticipated_debt // temp
    RESULT next_runnable_result

schedule_cpus():

foreach project P
    P.work_done_this_period = 0

total_work_done_this_period = 0
foreach task T that is RUNNING:
    x = T.current_cpu_time - T.period_start_cpu_time
    T.project.work_done_this_period += x
    total_work_done_this_period += x

foreach P in projects:
    P.debt += P.resource_share * total_work_done_this_period
            - P.work_done_this_period

expected_pay_off = total_work_done_this_period / num_cpus

foreach P in projects:
    P.anticipated_debt = P.debt

foreach task T
    T.next_scheduler_state = PREEMPTED

do num_cpus times:
    // choose the project with the largest anticipated debt
    P = argmax { P.anticipated_debt } over all P in projects with runnable result
    if none:
        break
    if (some T in P is RUNNING):
        T.next_scheduler_state = RUNNING
        P.anticipated_debt -= expected_pay_off
        continue
    if (some T in P is PREEMPTED):
        T.next_scheduler_state = RUNNING
        P.anticipated_debt -= expected_pay_off
        continue
    if (some R in results is for P, not active, and ready to run):
        Choose R with the earliest deadline
        T = new ACTIVE_TASK for R
        T.next_scheduler_state = RUNNING
        P.anticipated_debt -= expected_pay_off

foreach task T
    if scheduler_state == PREEMPTED and next_scheduler_state = RUNNING
        unsuspend or run
    if scheduler_state == RUNNING and next_scheduler_state = PREEMPTED
        suspend (or kill)

foreach task T
    T.period_start_cpu_time = T.current_cpu_time
</pre>

<h2>Work fetch policy</h2>

<p>
The work fetch policy has the following goal:

<ul>
<li>
Maintain sufficient work so that the CPU scheduler
is never 'starved' (i.e. a result is available for
a particular project when one is needed).
<li>
More specifically:
given a 'connection period' parameter T (days),
always maintain sufficient work so that the CPU scheduler will
not be starved for T days,
given average work processing.
The client should contact scheduling servers only about every T days.
<li>
Don't fetch more work than necessary, given the above goals.
Thus, try to maintain between T and 2T days worth of work.
</ul>

<h3>When to get work</h3>

<p>
The CPU scheduler needs a minimum number of results from a project
in order to respect the project's resource share.
This is the number of active tasks that a project can potentially have
running simultaneously, given its resource share.
We effectively have too little work when the number of results for a
project is less than this minimum number.

<blockquote>
min_results(P) = ceil(ncpus * P.resource_share)
</blockquote>

<p>
The client can estimate the amount of time that will elapse until we
have too little work for a project.
When this length of time is less than T, it is time to get more work.

<h3>A sketch of the work fetch algorithm</h3>

<p>
This algorithm determines if a project needs more work. If a project
does need work, then the amount of work it needs is computed.
It is called whenever the client can make a scheduler RPC.
<p>
<ol>
<li>
For each project
<ol>
<li>
If the number of results for the project is too few
<ol>
<li>
Set the project's work request to 2T
<li>
Set request urgency to NEED WORK IMMEDIATELY
<li>
Continue
</ol>
<li>
For all but the top (min_results - 1) results with the longest
expected time to completion:
<ol>
<li>
Sum the expected completion time of the result scaled by the work rate
and the project's resource share
</ol>
<li>
If the sum S is less than T
<ol>
<li>Set the project's work request to 2T - S
<li>Set request urgency to NEED WORK
</ol>
<li>
Else, set the project's work request to 0
</ol>
<li>
Return the request urgency
</ol>

<p>
The mechanism for actually getting work checks if a project has a
non-zero work request and if so, makes the scheduler RPC call to
request the work.

<h3>Pseudocode</h3>

<pre>
data structures:
PROJECT:
    double work_request
    double work_remaining // temp
RESULT:
    double cpu_time_remaining // temp

compute_work_request():

    foreach project P:
        P.work_request = 0
        P.work_remaining = 0

    foreach result R:
        R.cpu_time_remaining = estimate_cpu_time_remaining(R)

    // we ignore the top (min_results-1) by first subtracting
    // their cpu_time_remaining from the sum
    foreach project P:
        do min_results(P) - 1 times:
            R = argmax { R.cpu_time_remaining } over all results for P
            P.work_remaining -= R.cpu_time_remaining
            R.cpu_time_remaining = 0

    foreach result R:
        R.project.work_remaining += R.cpu_time_remaining

    foreach project P:
        if P.work_remaining < T:
            if P.work_remaining == 0:
                // no other results besides the top (min_results-1)
                need_work_immediately = 1
            P.work_request = 2*T - P.work_remaining / SECONDS_PER_DAY
            need_work = 1

    return need_work_immediately + need_work

</pre>

";
page_tail();
?>

