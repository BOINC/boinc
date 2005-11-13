<?php

require_once("docutil.php");

page_head("Statistics data");
echo "
<b>Statistics data</b> is the number of
<a href=credit.php>Cobblestones</a> (credit units)
granted to hosts, participants and teams by the
various BOINC projects.
This can be used for various purposes, such as:
<ul>
<li> 
Web sites that show statistics and leaderboards
for one or more BOINC projects.
Examples are listed at <a href=stats_sites.php>here</a>.

<li>  Dynamically-generated images
(typically used as message-board signatures)
that show user and/or team credit, possible across projects.
Examples are listed at <a href=stats_sites.php>here</a>.

<li> Displays of current credit on cell phones and PDAs.  </ul>

<p>
BOINC provides a flexible architecture for distributing
statistics data, with the goal of enabling new display applications.
<hr>
<center>
<img src=stats.png>
<br
<b>The BOINC statistics data architecture</b></center>
<hr>

<h2>Project-specific data</h2>
<p>
Each BOINC project provides data in two forms:
<ul>
<li> As
<a href=db_dump.php>a set of downloadable files</a>
(in a compressed XML format)
that contain the project's complete current statistics.
These files are typically updated once every 24 hours.
<li> As
<a href=web_rpc.php>a set of Web RPCs</a>
that return an XML-format description of a given
participant's credit,
based on that participant's database ID.
</ul>

<p>
Applications should access these data sources
as infrequently as possible,
to avoid imposing unnecessary load on project servers.
For example, a Web RPCs to get a particular participant's data
should made at most once per hour.


<h2>Aggregate data</h2>
<p>
A <b>data aggregator</b> is a service that collects XML files
from several projects,
and computes the totals for participants and hosts
(based on <a href=cpid.php>cross-project IDs</a>)
across these projects.
This aggregate data is then exported in two ways:
<ul>
<li> As
<a href=stats_xml.php>a set of downloadable files</a>
(in a compressed XML format).
An example is at  http://boinc.netsoft-online.com/stats/.
<li> As
<a href=stats_xml.php#rpc>a set of Web RPCs</a>
that return an XML-format description of a given participant
or host's credit,
based on the cross-project ID.
Example:
http://boinc.netsoft-online.com/get_host.php?cpid=????
and
http://boinc.netsoft-online.com/get_host_tot.php?cpid=????
</ul>
<p>

<p>
Application developers are encouraged to concentrate
on aggregate rather than per-project data.
This will encourage participants to join multiple projects,
and will make it possible for new projects to quickly get
many participants.
";

page_tail();

?>
