<?php

require_once("db_ops.inc");
require_once("util_ops.inc");

$cgi_url = parse_config("<cgi_url>");

db_init();

$w = 7*86400;
echo "
    <title>BOINC project management for ".PROJECT."</title>
    <h2>BOINC project management for ".PROJECT."</h2>
    <p>
    Browse database:
    <ul> 
    <li><a href=db_action.php?table=platform>Platforms</a>
    <li><a href=db_action.php?table=app>Applications</a>
    <li><a href=db_form.php?table=app_version>Application versions</a>
    <li><a href=db_form.php?table=user>Users</a>
    <li><a href=db_form.php?table=team>Teams</a>
    <li><a href=db_form.php?table=host&detail=low>Hosts</a>
    <li><a href=db_form.php?table=workunit>Workunits</a>
    <li><a href=db_form.php?table=result&detail=low>Results</a>
    </ul>
    </p>
    <p>
    Browse / Rate user profiles:
    <ul>
    <li><a href=profile_ops.php?set=unrated&num=0>Unrated profiles</a>
    <li><a href=profile_ops.php?set=approved&num=0>Approved profiles</a>
    <li><a href=profile_ops.php?set=rejected&num=0>Rejected profiles</a>
    <li><a href=profile_ops.php?set=uotd&num=0>Past Users of the Day</a>
    <li><a href=profile_ops.php?set=all&num=0>All profiles</a>
    </ul>
    </p>
";
$result = mysql_query("select * from app");
while ($app = mysql_fetch_object($result)) {
    echo "<br>Result summary for $app->name:
        <ul>
        <li><a href=result_summary.php?appid=$app->id&nsecs=86400>last 24 hours</a>
        <li><a href=result_summary.php?appid=$app->id&nsecs=$w>last week</a>
        </ul>
    ";
}
mysql_free_result($result);
echo "
    <a href=$cgi_url/stripchart.cgi>Stripcharts</a>
    | <a href=show_log.php>Show/Grep all logs</a>
    | <a href=show_log.php?f=mysql*.log&l=-20>Tail MySQL logs</a>
    | <a href=create_account_form.php>Create account</a>
";

// TODO: Add functionality to list the number of recommends / rejects received
// by each profiled user.

?>
