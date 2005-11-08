<?php

require_once("docutil.php");

page_head("Statistics data");
echo "
<b>Statistics data</b> is the number of
<a href=credit.php>Cobblestones</a> (credit units)
granted to hosts, participants and teams by the
various BOINC projects.
This data can be used for various purposes, such as:
<ul>
<li> Producing
web sites that show statistics and leaderboards
for one or more BOINC projects.
Examples are listed at <a href=stats_sites.php>here</a>.

<li> Producing dynamically-generated images
(typically used as message-board signatures)
that show user and/or team credit, possible across projects.
Examples are listed at <a href=stats_sites.php>here</a>.

<li> Producing displays of current credit on
cell phones and PDAs.
</ul>

<p>
BOINC provides a flexible architecture for distributing
statistics data, with the intent of enabling new uses of this data.
<p>
<img src=stats.png>
<p>
<center><b>The BOINC statistics data architecture</b></center>

<h2>Project data</h2>
<p>
Each BOINC project provides data in two forms:
<ul>
<li> As
<a href=db_dump.php>a set of downloadable files</a>
(in a compressed XML format)
that contain the project's complete current statistics.
These files are typically updated once every 24 hours.
<li> As
<a href=rpc.php>a set of Web RPCs</a>
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
(based on <a href=foo>cross-project IDs</a>)
across these projects.
This aggregate data is then exported in two ways:
<ul>
<li> As
<a href=stats_xml.php>a set of downloadable files</a>
(in a compressed XML format).
<li> As
<a href=stats_xml.php#rpc>a set of Web RPCs</a>
that return an XML-format description of a given participant
or host's credit,
based on the cross-project ID.
</ul>
<p>
Examples are here:
<a href=http://www.boinc.dk/index.php?page=download_stats_xml>
http://www.boinc.dk/index.php?page=download_stats_xml</a> and
<a href=http://www.boincstats.com/xml/xml_user_example.php>http://www.boincstats.com/xml/xml_user_example.php</a>.

<p>
Application developers are encouraged to concentrate
on aggregate rather than per-project data.
This will encourage participants to join multiple projects,
and will make it possible for new projects to quickly get
many participants.
";

page_tail();

?>
