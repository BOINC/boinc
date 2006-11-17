<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db_ops.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/uotd.inc");

$config = get_config();
$cgi_url = parse_config($config, "<cgi_url>");
$stripchart_cgi_url = parse_config($config, "<stripchart_cgi_url>");

db_init();

$title = "Project Management";
admin_page_head($title);

// Notification area
echo "<ul>\n";

if (!file_exists(".htaccess")) {
    echo "<li><span style='color: #ff0000'>The Project Management directory is not
        protected from public access by a .htaccess file.</span></li>\n";
}

if (parse_bool($config, "disable_account_creation")) {
    echo "<li><span style='color: #ff9900'>Account creation is disabled.</span></li>\n";
}

if (defined('INVITE_CODES')) {
    echo "<li><span style='color: #ff9900'>Account creation is restricted by the use of
	invitation codes.</span></li>\n";
}

$uotd_candidates = count_uotd_candidates();
if ($uotd_candidates >= 0) {
    if ($uotd_candidates >= UOTD_THRESHOLD*2) {
        $color = "#00aa00";
    } elseif ($uotd_candidates < UOTD_THRESHOLD) {
        $color = "#ff0000";
    } else {
        $color = "#ff9900";
    }
    echo "<li><span style='color: ".$color."'>There are ".$uotd_candidates." remaining
    candidates for User of the Day.</span></li>\n";
}

echo "</ul>\n";

echo "
    <p>
    <table border='0'><tr valign='top'>
    <td><b>Browse database:</b>
    <ul> 
        <li><a href=\"db_action.php?table=platform\">Platforms</a></li>
        <li><a href=\"db_action.php?table=app\">Applications</a></li>
	<li><a href=\"db_form.php?table=app_version\">Application versions</a></li>
	<li><a href=\"db_form.php?table=user\">Users</a></li>
	<li><a href=\"db_form.php?table=team\">Teams</a></li>
	<li><a href=\"db_form.php?table=host&detail=low\">Hosts</a></li>
	<li><a href=\"db_form.php?table=workunit\">Workunits</a></li>
	<li><a href=\"db_form.php?table=result&detail=low\">Results</a></li>
    </ul>

    </td> 
    <td><b>Regular Operations:</b>
    <ul>
	<li><a href=\"profile_screen_form.php\">Screen user profiles </a></li>
	<li><a href=\"create_account_form.php\">Create account</a></li>
	<li><a href='manage_special_users.php'>Manage special users</a></li>
    </ul>

    </td> 
    <td><b>Special Operations:</b>
    <ul>
	<li><a href='manage_apps.php'>Manage applications</a></li>
	<li><a href='manage_app_versions.php'>Manage application versions</a></li>
	<li><a href='mass_email.php'>Send mass email to a selected set of users</a></li>
	<li><a href=\"forum_repair.php\">Forum repair</a></li>
	<li><a href=\"repair_validator_problem.php\">Repair a validator problem</a></li>
	<li><a href=\"problem_host.php\">Email user with misconfigured host</a></li>
	<li><a href=\"cancel_wu_form.php\">Cancel workunits</a></li>
	<li>
	    <form method=\"get\" action=\"clear_host.php\">
	    Clear Host: 
	    <input type=\"text\" size=\"5\" name=\"hostid\">
	    <input type=\"submit\" value=\"Clear RPC\">
	    </form>
	</li>
    </ul>
    </td>
    </tr>
    </table>
";

// Application Result Summaries:

$show_deprecated = get_str("show_deprecated", true);
$show_only = array('all'); // Add all appid's you want to display, or 'all'
$result = mysql_query("select id, name, deprecated from app");
while ($app = mysql_fetch_object($result)) {
    if ( in_array($app->id, $show_only) 
       || ( in_array("all", $show_only)
          && (!$app->deprecated || $show_deprecated)
          )
       ) {

    echo "
	<b>Result summary for <tt>$app->name</tt>:</b>
	<ul>
    <li> Past 24 hours:
	 <a href='result_summary.php?appid=$app->id&nsecs=86400'>summary</a> |
	 <a href='pass_percentage_by_platform.php?appid=$app->id&nsecs=86400'>pass percentage by platform</a> | 
	 <a href='failure_result_summary_by_host.php?appid=$app->id&nsecs=86400'>failure by host</a> |
         <a href='failure_result_summary_by_platform.php?appid=$app->id&nsecs=86400'> failure by platform</a>
    <li>Past &nbsp;&nbsp;&nbsp;7 days:
	 <a href='result_summary.php?appid=$app->id&nsecs=604800'>summary</a> |
         <a href='pass_percentage_by_platform.php?appid=$app->id&nsecs=604800'>pass percentage by platform</a> |
         <a href='failure_result_summary_by_host.php?appid=$app->id&nsecs=604800'>failure by host</a> |
	 <a href='failure_result_summary_by_platform.php?appid=$app->id&nsecs=604800'>failure by platform</a>
      </ul>
	";
    }
 }
mysql_free_result($result);

if ($show_deprecated) {
    echo "<a href='index.php?show_deprecated=0'>Hide deprecated applications</a>";
} else {
    echo "<a href='index.php?show_deprecated=1'>Show deprecated applications</a>";
}

// Periodic tasks

echo "<h3>Periodic or special tasks</h3>
    <UL>
    <li> The following scripts should be run as periodic tasks,
    not via this web page
    (see <a href='http://boinc.berkeley.edu/project_tasks.php'
            target='_boincdoc'>http://boinc.berkeley.edu/project_tasks.php</a>):
     <pre> update_forum_activities.php, update_profile_pages.php, update_uotd.php</pre>
    <li> The following scripts can be run manually on the command line
    as needed (i.e. <tt>php scriptname.php</tt>):
        <pre>forum_repair.php, team_repair.php, repair_validator_problem.php</pre>
   </UL>
    ";


// Stripcharts, logs, etc

echo "<P>
    <a href='$stripchart_cgi_url/stripchart.cgi'>Stripcharts</a>
    | <a href='show_log.php'>Show/Grep all logs</a>
    | <a href='show_log.php?f=mysql*.log&l=-20'>Tail MySQL logs</a>
	<P>
	";

?>
