<?
require_once("docutil.php");
page_head("Handling long, large-footprint computations");
echo "
<p>
The sequential components of some applications are long
(weeks or months) and have a large data 'footprint',
i.e. their state may be many MB or GB in size.
Each component could be represented by a single workunit.
However, this has two potential drawbacks.
<ul>
<li> In the presence of participant fluctuation,
the fractions of uncompleted workunits
and wasted CPU time increase without bound.
<li> If hardware error occurs early in a workunit,
the rest of the CPU time is wasted.
<li> It's impossible for the project to abort a component
(e.g. because another component has a better result).
</ul>

<p>
Alternatively, each component could be subdivided into
a number of workunits of bounded average duration (e.g. one day).
This avoids the above problems, but it requires the state files
to be uploaded frequently, which may impose prohibitive
network and storage loads.

<p>
To solve these problems, BOINC provides a mechanism
called <b>work sequences</b>.
Each work sequence represents a long, large-footprint component.
It consists of a sequence W1, ... Wn of workunits;
each workunit has one or more results.
BOINC attempts to execute a work sequence
entirely on one host, since this minimizes network traffic.
However, it will 'relocate' a work sequence
(i.e. shift it to another host) if
<ul>
<li> The current host fails or is too slow
<li> The current host returns an erroneous result
</ul>

<p>
A work sequence is dynamic;
the project back end may extend it depending on the results.

<p>
The output files of a result in a sequence are classified as
<ul>
<li> State files: these may be used as input files of
later workunits in the sequence.
They are typically large.
They need not be uploaded.
<li> Answer files: these are a high-level summary of
the results thus far.
They are typically small and are always uploaded.
They allow the project to decide whether to extent the sequence.
</ul>
A result need not have ANY output files;
such a result supplies a 'heartbeat' telling the
server that the host is still working.

<h3>Creating work sequences</h3>
<p>
A project creates a work sequence using
the <b>create_work</b> utility.
This creates an initial sequence, which may be extended later (see below).
The sequence has one result per work unit.

<p>
The results in a sequence typically have the following structure:
<ul>
<li> Each Nth result generates and uploads an answer file.
N may be as small as one if frequent checking (for correctness or
comparison with other sequences) is needed.
<li> Each result generates a state file,
but only every Mth result uploads the file.
Some applications might have a 'hierarchical state' scheme in which,
for example, a large state file is uploaded every 50 results
while a smaller state file is uploaded very 10 units.
This could reduce wasted CPU for a given level of network traffic.
</ul>
The decision of whether to use work sequences,
and the optimal values of parameters (N, M, and workunit duration)
depend on many factors and are left up to the project.

<h3>Scheduling work sequences</h3>
<p>
A work sequence is represented in the BOINC database by its first workunit.
At a given time each sequence is either unassigned or is
assigned to a particular host.
Each host keeps track of the sequences it believes are assigned to it.
<p>
This head workunit contains a 'maximum result time';
the sequence should only be assigned to hosts that
can complete a result in this amount of real time.
It also contains a link to the first workunit in the sequence
for which no validated result has been received,
and to the first workunit that has not been dispatched yet
to the assigned host.
<p>
Each RPC to a BOINC scheduling server includes a list of
work sequences assigned to the host.
<p>
If no work sequences are assigned to the host,
the scheduler checks for an unassigned work sequence
that the host is fast enough to handle.
If there is one, it sends it one or more results from
this sequence, and assigns the sequence to the host.

<p>
Scheduling notes:
<li>
If the high-water mark allows multiple results to be active,
the host can upload result N while processing result N+1.
<li>
Conceivably there could be cases where a state file upload could
be aborted because a newer state file is available.
This isn't worth worrying about.
<li>
This policy limits each host to one sequence; may be want to
modify for multiprocessors.
</ul>

<p>
For each sequence reportedly assigned to the host,
the scheduler checks whether the sequence is still assigned to the host,
and if not returns an element telling the host it's no longer assigned.
Otherwise, it sends the host additional results from the sequence,
up to the limit specified by the request.
(Note: if the project has a mix of sequence and non-sequence work,
this will starve the non-sequence work for this host.)

<p>
<h3>Ending or extending a work sequence</h3>
<p>
The project's result-handling program,
when it has processed a completed result from a work sequence,
may generate additional work units and result in the sequence
(perhaps a single additional result that uploads the final state files)
or it may specify that the sequence has ended.
In the latter case, the scheduler will notify the host
that the sequence has ended, removing its assignment.

<h3>Relocating work sequences</h3>
<p>
A BOINC daemon process periodically checks for work sequences
for which some result, dispatched to the currently assigned host,
has missed its deadline.
It then deassigns the work sequence,
and prepares it for reassignment.
This involves the following:
<ul>
<li> Find the latest workunit Wi in the sequence
all of whose input files have been uploaded.
<li> Generate a new result for Wi
with different output filenames.
<li> For each workunit W after Wi in the sequence,
create a new workunit W' and a new result R,
changing the filenames as needed.
<li> Reset the pointers in the head workunit.
</ul>
This creates a 'dead branch' which remains in the database.

<h3>Redundancy checking for work sequences</h3>
<p>
Redundancy checking for work sequences requires some additional logic
because the corresponding groups of results
belong to different workunits.
The proposed mechanism is as follows:

<ul>
<li> Several work sequences with identical initial states
can be created and assigned to the same <b>work sequence group</b>.
One of the sequences is chosen as the 'master' and the others
are linked to it.

<li> When results for a minimum quorum of
the corresponding workunits in a group
(e.g. the 1st workunit in all the groups)
have been received, they are compared using the same
mechanism as for regular workunits.
This is repeated as new results are received until
a canonical result is found.

<li> If all the result are present and there is no canonical result,
the project is notified (this shouldn't happen often).

<li> Whenever an erroneous result is found,
the work sequence is deasssigned as described above.
</ul>

Note: this scheme could lead to situations in which
a slow host holds up the granting of credit to faster hosts.
May want to reassign to faster host in this case.

<h3>Examples</h3>
<p>
climateprediction.com (known computation length, large state files).
Let's say a simulation takes 6 months.
Suppose we want a small 
progress report from the user every 3 days, so we generate 6*30/3 = 60 
results per sequence.
The scheduling server ensures that each host has 
at least 2 elements of the sequence at a time, so that it doesn't have 
to wait for data upload in order to continue.
If we want a full state 
save every 3 weeks, we make every 7th result restartable and set the 
XML file infos so that the large state files will be uploaded.

<p>
Folding@home (unknown computation length, small state files).
We don't know in advance how long a computation will take.
The server generates a large group of trajectory sequences,
but only creates 2 or 3 results in each sequence.
The backend work generator periodically checks how 
much of each sequence has been completed, and extends any sequences 
that are nearing completion unless it has been decided to permanently 
terminate them (i.e. because a more promising trajectory has been found).
";
page_tail();
?>
