<?php
require_once("docutil.php");
page_head("Memory usage");
echo "
This document describes proposed policies and mechanisms
related to RAM and swap space.
There are several issues:
<ul>
<li> <b>Job dispatch</b>: how do we decide whether to send a job to a host,
based on the job's memory requirements and the host's memory resources?
<li> <b>Client CPU scheduling</b>: how do memory factors
affect when jobs should run?
<li> <b>Job abort policy</b>:
when must a job be aborted because it is using too much memory?
</ul>
<h3>Issues and goals</h3>
<p>
BOINC applications run at the lowest CPU priority.
However, they can impact user-visible performance
because of their memory usage:
<ul>
<li> When the system is in use (i.e. when there's mouse/keyboard input),
the memory usage of running BOINC apps can increase the level of paging,
making the system sluggish.
<li> If several user apps are open and the system is idle for a long period,
the memory usage of BOINC apps may cause the user apps to be paged out.
When the user eventually returns,
it may take a while (10-20 seconds) for the user apps to get paged back in.
</ul>

<p>
These effects can be minimized by limiting BOINC apps to
a very small amount of memory.
However, this reduces the CPU time available to BOINC,
and on some systems BOINC would do no work at all.
There is a tradeoff: the more work BOINC does,
the greater its potential impact on user-visible performance.
One goal of our design is to provide user-adjustable
controls (i.e. <a href=prefs.php>general preferences</a>) over this tradeoff.

<p>
A second goal is to maximize the CPU efficiency of BOINC apps,
i.e. to ensure that they don't thrash.
On a multiprocessor, it may sometimes be more efficient
(in terms of total CPU time per wall time)
to run fewer jobs than the number of CPUs.

<p>
A third goal is to support applications that are <b>memory-aware</b>,
i.e. that can trade off memory usage for speed.
Such applications should be made aware of the current
memory constraints, so that they can adapt accordingly.

<h3>Client data</h3>

When it starts up, BOINC measures:
<ul>
<li> The amount of RAM on the system.
<li> The amount of swap space.
On Win/Unix, this is the size of the page file or swap partition.
On Mac, it's the amount of free disk space.
</ul>
BOINC measures the following periodically (every 10 seconds or so):
<ul>
<li> For each executing BOINC app: the working set size
(for compound apps, this includes all processes).
The definition of 'working set' may vary between OSs,
but we assume that it means the amount of RAM needed
to run with high (say, > 90%) CPU utilization.
This is not necessarily the amount of RAM the app currently is using.
<p>
To accommodate spikes in memory usage, BOINC smooths the working set size:
the actual value used is computed as
<p>
WSS = .5*WSS + .5*WSS_OS
<p>
where WSS_OS is the working set size as reported by the OS.
<li> For each BOINC app: the amount of swap space used.
</ul>
Data we don't have:
<ul>
<li> Page-fault rates for each app.
This doesn't seem to be available on Win
(the reported page fault rate includes faults that
don't read from disk).
</ul>

<h3>Server data</h3>
Each workunit record includes:
<ul>
<li> <b>rsc_memory_bound</b>:
an estimate of the app's largest working set size.
</ul>

<h3>Preferences</h3>
We propose the following:
<ul>
<li> <b>ram_max_used_frac_busy</b>:
Max fraction of RAM to use while system is busy
<li> <b>ram_max_used_frac_idle</b>:
Max fraction of RAM to use while system is idle
<li> <b>vm_max_used_pct</b>:
Max percentage of swap space to use (this already exists)
</ul>

<h3>Scheduler (server side)</h3>
<p>
A result is sent to a client only if
<p>
rsc_memory_bound < (RAM size)*min(ram_max_used_frac_busy, ram_max_used_frac_idle)

<h3>Client CPU scheduler</h3>
The scheduler is divided into two parts:
<ul>
<li> Make a list of tasks to run,
ordered by 'importance' (deadline-critical ones first,
then high-debt).
<li> Enforcement: go through the run list,
starting tasks in order, and preempting other tasks as needed.
Don't preempt a task that hasn't checkpointed in favor
of a non-deadline-critical task.
</ul>
This will be modified as follows:
<ul>
<li> In building the run list,
compute the available RAM, based on preferences.
In building the list, keep track of RAM used so far.
Skip any task that would cause this to exceed available RAM.
<li> Enforcement:
compute the available RAM, based on preferences.
In running tasks, keep track of RAM used so far.
Skip any task that would cause the limit to be exceeded.
Preempt tasks that haven't checkpointed
if they would cause the limit to be exceeded.
</ul>

In addition, we will add a new 'memory usage check'
that runs every 30 seconds or so.
This will compute the working sets of all running tasks.
If the total is too large, it will trigger CPU scheduler enforcement
(see above).
If an individual task's working set is too large for it to
every run, it is aborted (see below).

<p>
Note: the above policies may cause some tasks to
not get run for long periods.
For example, suppose that
<ul>
<li> A 2-CPU machine has 1 GB RAM,
<li> There's a small-RAM job X with a close deadline
<li> There's a 1 GB job Y
<li> There are several small-RAM jobs.
</ul>
In this case, Y won't run until X has finished,
even if it more deserving (in terms of debt) than the other small jobs.
However, Y won't starve indefinitely.
Eventually it will run into deadline trouble,
and will run ahead of everything else.

<h3>Aborting tasks</h3>

A task is aborted if, at any point, its working set size is larger than
<p>
    (RAM size)*max(ram_max_used_frac_busy, ram_max_used_frac_idle)
<p>
since this means it can't be scheduled.

<h3>Memory-aware applications</h3>
<p>
The following items will be added to the BOINC_STATUS structure:
<pre>
double working_set_size;        // app's current WS (non-smoothed)
double max_working_set_size;    // app will be aborted if WS exceeds this
</pre>


<h3>Future work</h3>
<ul>
<li> Measure, and take into account, non-BOINC RAM usage.
Maybe the best policy is:
if non-BOINC RAM usage is X, BOINC uses total-X.
If the computer is busy
<li> Enforce bounds on swap space usage.
<li> Make the round-robin simulator aware of memory issues.
In the scenario described under Client CPU Scheduler,
the large-RAM task won't get classified as being in deadline trouble
until somewhat too late.
</ul>
";

?>
