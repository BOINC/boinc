<?php
require_once("docutil.php");

page_head("Locality scheduling");

echo "
<b>Locality scheduling</b> is intended for projects for which
<ul>
<li> Each workunit has a large input file
(it may have other smaller input files as well).
<li> Each large input file is used by many workunits.
</ul>

The goal of locality scheduling is to minimize
the amount of data transfer to hosts.
In sending work to at given host,
the scheduler tries to send results
that use input files already on the host.

<p>
To use locality scheduling, projects must do the following:

<ul>
<li> Workunit names must be of the form FILENAME__*,
where FILENAME is the name of the large input file
used by that workunit.
These filenames cannot contain '__'.
<li> The &lt;file_info> for each large input file must contain the tags
",html_text("   <sticky/>
   <report_on_rpc/>"),"
<li> The config.xml file must contain",html_text("<locality_scheduling/>"),"
</ul>

<p>
Locality scheduling works as follows:
<ul>
<li> Each scheduler RPC contains a list of the
large files already on the host, if any.
<li> The scheduler attempts to send results that use a file
already on the host.
<li> For each file that is on the host and for which
no results are available for sending,
the scheduler instructs the host to delete the file.
</ul>

<h2>On-demand work generation</h2>
<p>
This mechanism, which is used in conjunction with locality scheduling,
lets a project create work in response to scheduler requests
rather than creating all work ahead of time.
The mechanism is controlled by an element in config.xml of the form:
".html_text("
<locality_scheduling_wait_period> N </locality_scheduling_wait_period>
")."
where N is some number of seconds.

<p>
When a host storing file X requests work,
and there are no available results using X,
then the scheduler touches a 'trigger file'
<pre>
PROJECT_ROOT/locality_scheduling/need_work/X
</pre>
The scheduler then sleeps for N seconds, and makes one additional attempt
to find suitable unsent results.

<p>
The project must supply a 'on-demand work generator' daemon program
that scans the need_work directory.
If it finds an entry,
it creates additional workunits for the file,
and the transitioner then generates results for these workunits.
N should be chosen large enough so that both tasks
complete within N seconds most of the time
(10 seconds is a good estimate).

<p>
The work generator should delete the trigger file
after creating work.

<p>
In addition, if the work generator (or some other project daemon)
determines that no further workunits can be made for a file X,
then it can touch a trigger file
<pre>
PROJECT_ROOT/locality_scheduling/no_work_available/X
</pre>
If the scheduler finds this trigger file then it assumes that the project cannot
create additional work for this data file and skips the 'notify,
sleep, query again' sequence above.
Of course it still does the initial query,
so if the transitioner has made some new results for an existing
(old) WU, they will get picked up.

// Implementation notes:
// Work is organized in a hierarchy: File -> workunit -> result
// Let's say there are N active hosts and target_nresults=4.
// Optimally, we'd like to send each file to 4 hosts,
// and have them process all the results for that file.
//
// If the one_result_per_user_per_wu rule is in effect,
// a file may have work but be "excluded" for a particular user.
//
// Assigning work to a host with no files:
// - maintain a working set of N/4 files
// - when a host with no file requests work,
//   choose a file F uniformly (randomly or sequentially) from the working set.
// - if F is excluded for this user,
//   choose a file using a deterministic algorithm
//   that doesn't involve the working set
//   (don't want to do this in general to avoid flocking)
//
// The working set is represented by a directory
//    PROJECT/locality_scheduling/file_working_set/
// whose contents are names of files in the working set.
// A project-specific 'working set manager' daemon
// is responsible for maintaining this.
//
// If the scheduler finds that there are no sendable results for a file,
// it makes a file with that name in
//   PROJECT/sched_locality/files_no_work/
// The working set manager should poll this directory
// and remove those files from the working set.
// NOTE: BOINC may later create more results for the file,
// so it may be necessary to add it to the working set again.
//
// Assigning work to a host with a file F:
// - send more results for file F.
//   To do this efficiently, we maintain the following invariant:
//   For a given user/file pair, results are sent in increasing ID order.
//
// Some projects may want to generate work incrementally.
// They can do this by supplying a 'work generator' daemon
// that polls the directory PROJECT/locality_scheduling/need_work/
// and creates work for any filenames found there.
// To enable this, add the element <locality_scheduling_wait_period>
// to config.xml; this tells the scheduler how long to wait for
// work to appear.
//
// NOTE: we assume that all results have app_versions for the same
// set of platforms.  So if any result is rejected for this reason,
// we give up immediately instead of scanning everything.

";
page_tail();
?>
