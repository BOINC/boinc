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
that uses input files already on the host.

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

";
page_tail();
?>
