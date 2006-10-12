<?php
require_once("docutil.php");
page_head("Low-latency computing");
echo "
BOINC was originally designed for high-throughput computing,
and one of its basic design goals was
to minimize the number of scheduler RPCs
(in order to reduce server load and increase scalability).
In particular, when a client requests work from a server
and there is none, the client uses exponential backoff,
up to a maximum backoff off 1 day or so.
This policy limits the number of scheduler requests
to (roughly) one per job.
<p>
However, this backoff policy is inappropriate for
<b>low-latency</b> computing,
by which we main projects whose tasks must be completed
in a few minutes or hours.
Such projects require a <b>minimum connection rate</b>,
rather than seeking to minimize the connection rate.

<p>
For example, if you need to get batches of 10,000 jobs
completed with 5 minute latency,
and each job takes 2 minutes of computing,
you'll need to arrange to get 10,000 scheduler requests every 3 minutes
(and you'll need a server capable to handling this request rate).

<h3>The minimum connection rate</h3>
<p>
Suppose that, at a given time, the project has N hosts online,
and that each host has 1 CPU that computes at X FLOPS.

<p>
Suppose that the project's work consists of 'batches' of M jobs.
Each batch is generated at a particular time,
and all the jobs must be completed within time T.
For simplicity, assume that a batch is not created until
the previous batch has been completed,
and that each has is given at most one job from each batch.
Suppose that each job takes Y seconds to complete on
the representative X-FLOPS CPU.
<p>
Clearly, for feasibility we must have Y <= T and N >= M.
Let W = T - Y; a job must be dispatched within W seconds
if it is to be completed within T.
<p>
Now suppose that each host requests work every Z seconds.
Assume Z is small enough so that at least M requests
arrive in any given period of length W.
(TODO: figure out what this is, given a Poisson arrival process).

<p>
Then, within W second of the batch creation,
all of the jobs have been sent to hosts,
and within T seconds
(assuming no errors or client failures)
they have been completed and reported.
Note: this is a simplistic analysis, and doesn't take into account
multiprocessors, hosts of different CPU speed,
the possibility of sending multiple jobs to one client,
the ability for Z to vary between hosts,
and probably many other factors.
If someone wants to analyze this in more generality, please do!

<h3>How to do low-latency computing</h3>
<p>
The key component in the above is the ability to control Z,
the time between requests for a given host.
Starting with version 5.6 of the BOINC client,
it is now possible to control this:
each scheduler reply can include a tag".html_text("<next_rpc_delay>x</next_rpc_delay>")."
telling the client when to contact the scheduler again.
By varying this value, a project can achieve a rate of
connection requests necessary to achieve its latency bounds.

<p>
The current BOINC scheduler code has no support
for sending this tag, or for figuring out what its value should be.
If you want to do low-latency computing,
the scheduler must be modified and extended as follows:
<ul>
<li> Keep track of how many active hosts you have
(this will change over time).
<li> Keep track of the performance statistics of these hosts
(means and maybe variances of their FLOPS, for example).
<li> Parameterize your workload: how many jobs per batch,
latency bound, FLOPs per job, etc.
<li> Do the math and write the code for figuring out that
the RPC delay should be for a given host
(this is dynamic - it will change with the number of active hosts).
<li> Change the scheduler so that it sends this value in the reply,
and so that it sends the appropriate number of jobs.
</ul>

<p>
If you're interested in helping add these features to BOINC,
please <a href=contact.php>contact us</a>.
";
page_tail();
?>
