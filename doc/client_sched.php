<?php
require_once("docutil.php");
page_head("Result scheduling");
echo "

This document describes BOINC's policies for the following:

<ul>
<li> CPU scheduling policy: what result to run when.
<li> Work fetch policy: when to contact scheduling servers,
and which one(s) to contact.
</ul>

<h2>CPU scheduling</h2>

<p>CPU scheduling aims to achieve the following goals
(decreasing priority):</p>

<ol>

<li>
<b>Maximize CPU utilization</b>

<li>
<b>Respect the resource share allocation for each project.</b>
A project's resource share represents how much computing resources
(CPU time, network bandwith, storage space) a user wants to allocate
to the project relative to the resources allocated to all of the other
projects in which he is participating. The client should respect this
allocation to be faithful to the user. In the case of CPU time, the
result computation scheduling should achieve the expected time shares
over a reasonable time period.

<li>
<b>Satisfy result deadlines if possible.</b>

<li>
<b>Given a 'minimum variety' parameter MV (seconds),
reschedule CPUs at least once every MV seconds.</b>
The motivation for this goal stems from the potential
orders-of-magnitude differences in expected completion time for
results from different projects. Some projects will have results that
complete in hours, while other projects may have results that take
months to complete.  A scheduler that runs result computations to
completion before starting a new computation will keep projects with
short-running result computations stuck behind projects with
long-running result computations. A participant in multiple projects
will expect to see his computer work on each of these projects in a
reasonable time period, not just the project with the long-running
result computations.

<li>
<b>Minimize mean time to completion for results.</b>
This means that the number of active result computations for a project should be minimized.
For example, it's better to have one result from
project P complete in time T than to have two results from project P
simultaneously complete in time 2T.

</ol>

<p>
A result is 'active' if there is a slot directory for it.
A consequence of result preemption is that there can
be more active results than CPUs.


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
We decrease the debt to A by 20 minutes and increase it by 30 minutes (75% of 25 + 15).
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
    double cpu_at_last_schedule_point
    double current_cpu_time
    scheduler_state:
        PREEMPTED
        RUNNING
    next_scheduler_state    // temp
PROJECT:
    double work_done_this_period    // temp
    double debt
    double anticipated_debt // temp
    bool has_runnable_result

schedule_cpus():

foreach project P
    P.work_done_this_period = 0

total_work_done_this_period = 0
foreach task T that is RUNNING:
    x = current_cpu_time - T.cpu_at_last_schedule_point
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
        t.next_scheduler_state = RUNNING
        P.anticipated_debt -= expected_pay_off
        continue
    if (some T in P is PREEMPTED):
        T.next_scheduler_state = RUNNING
        P.anticipated_debt -= expected_pay_off
        continue
    if (some R in results is for P, not active, and ready to run):
        T = new ACTIVE_TASK for R
        T.next_scheduler_state = RUNNING
        P.anticipated_debt -= expected_pay_off

foreach task T
    if scheduler_state == PREEMPTED and next_scheduler_state = RUNNING
        unsuspend or run
    if scheduler_state == RUNNING and next_scheduler_state = PREEMPTED
        suspend (or kill)

foreach task T
    T.cpu_at_last_schedule_point = current_cpu_time
</pre>

<h2>Work fetch policy</h2>

<p>
The work fetch policy has the following goal:

<ul>
<li>
<b>Given a 'connection frequency' parameter 1/T (1/days), have enough
work for each project to meet CPU scheduling needs for T days.</b>
The client should expect to contact scheduling servers only every T
days.
So, it should try to maintain between T and 2T days worth of work.
</ul>

<h3>When to get work</h3>

<p>
The CPU scheduler needs a minimum number of results from a project
in order to respect the project's resource share.
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
Return NEED WORK IMMEDIATELY
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
<li>Return NEED WORK
</ol>
<li>
Else, set the project's work request to 0 and return DON'T NEED WORK
</ol>
</ol>

<p>
The mechanism for actually getting work checks if a project has a
non-zero work request and if so, makes the scheduler RPC call to
request the work.

<h3>Pseudocode</h3>

<pre>
data structures:
PROJECT:
    double work_request_days

check_work_needed(Project P):

if num_results(P) < min_results(P):
    P.work_request_days = 2T
    return NEED_WORK_IMMEDIATELY

top_results = top (min_results(P) - 1) results of P by expected
completion time

work_remaining = 0
foreach result R of P that is not in top_results:
    work_remaining += R.expected_completion_time
work_remaining *= P.resource_share * active_frac / ncpus

if work_remaining < T:
    P.work_request_days = 2T - work_remaining / seconds_per_day
    return NEED_WORK
else:
    P.work_request_days = 0
    return DONT_NEED_WORK


</pre>

";
page_tail();
?>

