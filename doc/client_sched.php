<?php
require_once("docutil.php");
page_head("Result scheduling");
echo "
<h2>Goals and motivation</h2>

<p>The BOINC client result computation scheduling aims to achieve the
following goals:</p>

<ol>

<li><b>Efficiently and effectively use system resources.</b><br>This
is clearly desirable.</li>

<li><b>Maintain a sense of minimum variety of projects for which
results are computed in a given amount of time.</b><br>A user
participating in multiple projects can get bored seeing his computer
work only on one project for a long time.</li>

<li><b>Respect the resource share allocation for each
project.</b><br>The user specifies the resource shares and thus
expects them to be honored.</li>

</ol>

<p>The motivation for the second goal stems from the potential
orders-of-magnitude differences in expected completion time for
results from different projects. Some projects will have results that
complete in hours, while other projects may have results that take
months to complete.  A scheduler that runs result computations to
completion before starting a new computation will keep projects with
short-running result computations stuck behind projects with
long-running result computations. A participant in multiple projects
will expect to see his computer work on each of these projects in a
reasonable time period, not just the project with the long-running
result computations.</p>

<p>A project's resource share represents how much computing resources
(CPU time, network bandwith, storage space) a user wants to allocate
to the project relative to the resources allocated to all of the other
projects in which he is participating. The client should respect this
allocation to be faithful to the user. In the case of CPU time, the
result computation scheduling should achieve the expected time shares
over a reasonable time period.</p>

<p>At the same time, the scheduler RPC policy needs to complement the
result scheduling. We have the following goals for this policy:</p>

<ol>

<li><b>Have enough work to keep all CPUs busy</b></li>

<li><b>Have enough work to provide for minimum variety of
projects</b></li>

<li><b>Respect work_buf_min and work_buf_max</b></li>

</ol>

<h2>BOINC client result computation scheduling</h2>
 
<p>We address the goals using result preemption. After a given time
period, the client decides on a new set of projects for which results
will be computed in the next time period. This decision will consider
the projects' resource shares by tracking the debt owed to a project.
The debt to a project accrues according to the project's resource
share, and is paid off when CPU time is devoted to the project.</p>

<p>A consequence of result preemption is that projects can have
multiple active result computations at a given time. For example,
consider a two processor system participating in two projects, A and
B, with resource shares 75% and 25%, respectively. Ideally, one
processor will run a result computation for A, while the other
processor will switch between running result computations for A and B.
Thus, A will have two active result computations.  This consequence
implies a desirable property of the result preemption scheme: <b>that
the number of active result computations for a project be
minimized.</b> For example, it's better to have one result from
project P complete in time T than to have two results from project P
simultaneously complete in time 2T.  Maintaining more active result
computations than necessary increases the mean-time-to-completion if
the client switches between these active result computations.</p>

<p>We will attempt to minimize the number of active result
computations for a project by dynamically choosing results to compute
from a global pool.  When we allocate CPU time to project, we will
choose results to compute intelligently: choose already running tasks
first, then preempted tasks, and only choose to start a new result
computation in the last resort. This will not guarantee the above
property, but we hope it will be close to achieving it.</p>

<h3>A sketch of the result preemption algorithm</h3>

<p>The algorithm requires that a time period length be defined (e.g.
one hour).  The result preemption algorithm is run at the beginning of
each period. It proceeds as follows:</p>

<ol>

<li>Pay off debts to projects according to the amount of work done for
the projects in the last period.</li>

<li>Accrue debts to projects according to the projects' resource
shares.</li>

<li>Let the expected future debt for each project be initialized to
its actual debt.</li>

<li>Repeat until we decide on a result to compute for each
processor:</li>

<ol>

<li>Choose the project that has the largest expected future debt and a
ready-to-compute result.</li>

<li>Decrease the expected future debt of the project by the amount we
expect to pay off, and return the project back into consideration for
running on another processor.</li>

</ol>

<li>Preempt the current result computations with the new ones.</li>

</ol>

<p>Because result computations may finish before the time period
expires, we need to account for such a gap in a project's debt
payment. So, we need to also keep track of the amount of work done
during the current time period for each project as results finish.
This accounting should be reset for each time period.</p>

<p>Finally, the starting of new result computations in the middle of a
time period needs to use this accounting instead of the expected
future debts that were estimated at the beginning of the time period.
Otherwise, it will be similar to the decision of choosing which tasks
to run at the beginning of a time period.</p>

<h3>Pseudocode</h3>

<p>We'll initialize <tt>total_work_done_this_period</tt> to
<tt>num_cpus * period_length</tt>.</p>

<pre>
preempt_apps(): // called after every period_length

// finish accounting
foreach T in running_tasks:
    T.project.work_done_this_period += T.work_done_this_period
    total_work_done_this_period += T.work_done_this_period

// pay off and accrue debts
foreach P in projects:
    P.debt += P.resource_share * total_work_done_this_period
            - P.work_done_this_period

// make preemption decisions
expected_pay_off = total_work_done_this_period / num_cpus
foreach P in projects:
    P.expected_future_debt = P.debt
to_preempt.addAll(running_tasks) // assume we'll preempt everything at first
to_run = ()
do num_cpus times:
    found = false
    do projects.size times:
        // choose the project with the largest expected future debt
        P = argmax { P.expected_future_debt } over all P in projects
        if (some T in to_preempt is for P):
            // P has a task that ran last period, so just don't preempt it
            to_preempt.remove(T)
            T.expected_pay_off = expected_pay_off
            found = true
            break
        if (some T in preempted_tasks is for P):
            // P has a task that was preempted
            preempted_tasks.remove(T)
            to_run.add(T)
            T.expected_pay_off = expected_pay_off
            found = true
            break
        if (some R in results is for P, not active, and ready to run):
            T = new ACTIVE_TASK for R
            to_run.add(T)
            T.expected_pay_off = expected_pay_off
            found = true
            break
        remove P from consideration in the argmax
    if found:
        P.expected_future_debt -= expected_pay_off
    else:
        break
suspend tasks in to_preempt (reset T.expected_pay_off for each T in to_preempt)
run or unsuspend tasks in to_run (and put in running_tasks)

// reset accounting
foreach P in projects:
    P.work_done_this_period = 0
total_work_done_this_period = 0

----------

start_apps(): // called at each iteration of the BOINC main loop

foreach P in projects:
    // expected_future_debt should account for any tasks that finished
    // and for tasks that are still running
    P.expected_future_debt = P.debt - P.work_done_this_period
foreach T in running_tasks:
    T.project.expected_future_debt -= T.expected_pay_off

to_run = ()
while running_tasks.size &lt; num_cpus:
    do projects.size times:
        // choose the project with the largest expected future debt
        P = argmax { P.expected_future_debt } over all P in projects
        if (some T in preempted_tasks is for P):
            // P has a task that was preempted
            preempted_tasks.remove(T)
            to_run.add(T)
            T.expected_pay_off = fraction_of_period_left * expected_pay_off
            found = true
            break
        if (some R in results is for P, not active, and ready to run):
            T = new ACTIVE_TASK for R
            to_run.add(T)
            T.expected_pay_off = fraction_of_period_left * expected_pay_off
            found = true
            break
        remove P from consideration in the argmax
    if found:
        P.expected_future_debt -= fraction_of_period_left * expected_pay_off
    else:
        break
run or unsuspend tasks in to_run

----------

handle_finished_apps(): // called at each iteration of the BOINC main loop

foreach T in running_tasks:
    if T finished:
        // do some accounting
        T.project.work_done_this_period += T.work_done_this_period
        total_work_done_this_period += T.work_done_this_period
        do other clean up stuff

</pre>

<h3>Debt computation</h3>

<p>The notion of debt is used to respect the resource share allocation
for each project. The debt to a project represents the amount of work
(in CPU time) we owe to a project. Debt is paid off when CPU time is
devoted to a project. We accrue the debt to a project according to the
total amount of work done in a time period scaled by the project's
resource share.</p>

<p>For example, consider a system participating in two projects, A and
B, with resource shares 75% and 25%, respectively. Suppose in some
time period, the system devotes 25 minutes of CPU time to project A
and 15 minutes of CPU time to project B. We decrease the debt to A by
20 minutes and accrue it by 30 minutes (75% of 25 + 15). So the debt
increases overall. This makes sense because we expected to devote a
larger percentage of the system resources to project A than it
actually got.</p>

<p>The choosing of projects for which to start result computations at
the beginning of each time period can simply follow the debt ordering
of the projects. The algorithm computes the expected future debt to a
project (the debt we expect to owe after the time period expires) as
it chooses result computations to run.</p>

<blockquote>expected future debt = debt - expected pay off * number of
tasks to run this period</blockquote>

<p>However, choosing projects to run in the middle of a time period is
a little different. The preemption algorithm expected each of the
tasks it started to last for the entire time period. However, when a
task finishes in the middle of a time period, the expected future debt
to the respective project is an overestimate. We thus change the
expected future debt to reflect what has taken place: it is the debt
owed to the project at the beginning of the time period, minus the
amount of work that has already been done this time period, and minus
the amount of work we expect to complete by the end of the time
period. When projects have results chosen to run, we decrease the
expected future debt by the amount of work we expect to be done for
the project in the remainder of the time period.</p>

<blockquote>expected future debt = debt - (work completed + expected
pay off of tasks already running this period + expected pay off *
fraction of period left * number of new tasks for this
period)</blockquote>

<h2>Scheduler RPC policy</h2>

<p>The client should get more work when either of the following are
true:</p>

<ol>

<li>The client will have no work in at most work_buf_min days.</li>

<li>The client will not have enough work for a project to get its fair
share of computation time (according to its resource share)</li>

<li>The client will have fewer than num_cpus tasks</li>

</ol>

<p>Ignoring the second case can cause long running result computations
to monopolize the CPU, even with result preemption. For example,
suppose a project has work units that finish on the order of months.
Then, when work_buf_min is on the order of days, the client will never
think it is out of work. However, projects with shorter result
computations may run out of work. So, even with preemption, we cannot
have minimum variety.</p>

<h3>need_to_get_work()</h3>

<p>The second case (running out of work for one project) is addressed
by capping the amount of work counted for a project. We cap it by the
total amount of work that can be done in min_work_buf_secs, scaled by
the project's resource share. Thus, the client will get more work when
any one project has too little work.</p>

<p>The case of having fewer results than CPUs is addressed by
\"packing\" results into CPU \"bins\".</p>

<pre>
need_to_get_work():

    num_cpus_busy = 0
    total_work_secs = 0
    work_secs_for_one_cpu = 0
    foreach P in projects:
        P.avail_work_secs = 0

    sort results in order of decreasing estimated_cpu_time

    // pack results into CPU bins
    foreach R in results:
        result_work_secs = estimated_cpu_time(R)
        work_secs_for_one_cpu += result_work_secs
        R.project.avail_work_secs += result_work_secs
        if work_secs_for_one_cpu &gt;= min_work_buf_secs
            work_secs_for_one_cpu = 0
            num_cpus_busy += 1

    // count total amount of work, but cap amount any one project contributes
    // to this total
    foreach P in projects:
        total_work_secs += min { P.avail_work_secs,
                            P.resource_share * min_work_buf_secs * num_cpus }

    return (num_cpus_busy &lt; num_cpus)
        || (total_work_secs &lt; min_work_secs * num_cpus)
</pre>

<p>XXX it will be useful to know what caused this predicate to return
true, so maybe it should be split into separate predicates.</p>

<p>XXX also need to factor in if we are able to currently contact a
project (according to min_rpc_time).</p>
";
page_tail();
?>

