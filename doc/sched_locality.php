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
";
page_tail();
?>
