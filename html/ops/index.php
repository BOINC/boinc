<?php

require_once("util.inc");

$cgi_url = parse_config("<cgi_url>");

echo <<<EOT
<title>BOINC Project Management</title>
<h2>BOINC Project Management</h2>
<p>
Browse database:
<ul> 
<li><a href=db_action.php?table=platform>Platforms</a>
<li><a href=db_form.php?table=app>Applications</a>
<li><a href=db_form.php?table=app_version>Application versions</a>
<li><a href=db_form.php?table=user>Users</a>
<li><a href=db_form.php?table=team>Teams</a>
<li><a href=db_form.php?table=host>Hosts</a>
<li><a href=db_form.php?table=workunit>Workunits</a>
<li><a href=db_form.php?table=result&detail=low>Results</a>
</ul>
<a href=result_summary.php?nsecs=86400>Result summary, last 24 hours</a>
<br>
<a href=$cgi_url/stripchart.cgi>Stripcharts</a>

EOT;

?>
