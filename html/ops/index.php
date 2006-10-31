<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db_ops.inc");
require_once("../inc/util_ops.inc");

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

echo "</ul>\n";

echo "
    <p><table><tr valign=\"top\"><td width=\"30%\">
    Browse database:
    <ul> 
    <li><a href=\"db_action.php?table=platform\">Platforms</a>
    <li><a href=\"db_action.php?table=app\">Applications</a>
    <li><a href=\"db_form.php?table=app_version\">Application versions</a>
    <li><a href=\"db_form.php?table=user\">Users</a>
    <li><a href=\"db_form.php?table=team\">Teams</a>
    <li><a href=\"db_form.php?table=host&detail=low\">Hosts</a>
    <li><a href=\"db_form.php?table=workunit\">Workunits</a>
    <li><a href=\"db_form.php?table=result&detail=low\">Results</a>
    </ul></td>
    <td width=\"50%\">
    Maintain and Modify database:
    <ul>
    <li><a href=\"profile_screen_form.php\">Screen user profiles for User of the Day</a>
    <li><a href=\"create_account_form.php\">Create account</a>
    <li><a href=\"cancel_wu_form.php\">Cancel workunits</a>
    <li><a href=\"manage_special_users.php\">Manage special users</a>
    <li><a href=\"problem_host.php\">Email user with misconfigured host</a>
    <li><a href=\"mass_email.php\">Send mass email to a selected set of users</a>
    <li> The following scripts should be run as periodic tasks,
    not via this web page
    (see http://boinc.berkeley.edu/project_tasks.php):
    update_forum_activities.php,
    update_profile_pages.php, update_uotd.php.
    <li> The following scripts can be run manually on the command line
    as needed (i.e. php script.php):
    forum_repair.php, team_repair.php, repair_validator_problem.php.


    </ul>
    </table>
";

echo "<br><form method=\"get\" action=\"clear_host.php\">
    Clear Host: 
    <input type=\"text\" size=\"5\" name=\"hostid\">
    <input type=\"submit\" value=\"Clear RPC\">
    </form>
    ";


$show_only = array('all'); // Add all appid's you want to display or 'all'
$result = mysql_query("select id, name from app");
while ($app = mysql_fetch_object($result)) {
    if (in_array($app->id, $show_only) || in_array("all", $show_only)) {
    echo "<br>Result summary for $app->name:
        <ul>
        <li><a href=\"result_summary.php?appid=$app->id&nsecs=86400\">last 24 hours</a>
        <li><a href=\"pass_percentage_by_platform.php?appid=$app->id&nsecs=86400\">last 24 hours - pass percentage by platform</a>
        <li><a href=\"failure_result_summary_by_host.php?appid=$app->id&nsecs=86400\">last 24 hours - failure by host</a>
        <li><a href=\"failure_result_summary_by_platform.php?appid=$app->id&nsecs=86400\">last 24 hours - failure by platform</a>
        <li><a href=\"result_summary.php?appid=$app->id&nsecs=604800\">last week</a>
        <li><a href=\"pass_percentage_by_platform.php?appid=$app->id&nsecs=604800\">last week - pass percentage by platform</a>
        <li><a href=\"failure_result_summary_by_host.php?appid=$app->id&nsecs=604800\">last week - failure by host</a>
        <li><a href=\"failure_result_summary_by_platform.php?appid=$app->id&nsecs=604800\">last week - failure by platform</a>
        </ul>
    ";
    }
}
mysql_free_result($result);

echo "<br>\n";
echo "
    <a href=\"$stripchart_cgi_url/stripchart.cgi\">Stripcharts</a>
    | <a href=\"show_log.php\">Show/Grep all logs</a>
    | <a href=\"show_log.php?f=mysql*.log&l=-20\">Tail MySQL logs</a>
    | <a href=\"errorwus.php\">List all-error Workunits</a>
";

admin_page_tail();
// TODO: Add functionality to list the number of recommends / rejects received
// by each profiled user.

?>
