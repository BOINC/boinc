<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/db_ops.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/uotd.inc");
require_once("../project/project.inc");

$config = get_config();
$stripchart_cgi_url = parse_config($config, "<stripchart_cgi_url>");

db_init();

$title = "Project Management";
admin_page_head($title);

// Notification area
echo "<ul>\n";

if (!file_exists(".htaccess")) {
    echo "<li><span style=\"color: #ff0000\">
        The Project Management directory is not
        protected from public access by a .htaccess file.
        </span></li>
    ";
}

if (!defined("SYS_ADMIN_EMAIL")) {
    echo "<li><span style=\"color: #ff0000\">
        The defined constant SYS_ADMIN_EMAIL
        has not been set. Please edit <tt>project/project.inc</tt> and set this
        to an address which can be used to contact the project administrators.
        </span></li>
    ";
}

if (parse_bool($config, "disable_account_creation")) {
    echo "<li><span style=\"color: #ff9900\">
        Account creation is disabled.</span></li>
    ";
}

if (defined("INVITE_CODES")) {
    echo "<li><span style=\"color: #ff9900\">
        Account creation is restricted by the use of
        invitation codes.</span></li>
    ";
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
    echo "<li><span style=\"color: ".$color."\">
        There are ".$uotd_candidates." remaining candidates for User of the Day.
        </span></li>
    ";
}

echo "</ul>\n";

if (function_exists('admin_index_extra')) {
    admin_index_extra();
}

echo "
    <p>
    <table border=\"0\"><tr valign=\"top\">
    <td><b>Browse database:</b>
    <ul>
        <li><a href=\"db_form.php?table=result&amp;detail=low\">Results</a></li>
        <li><a href=\"db_form.php?table=workunit\">Workunits</a></li>
        <li><a href=\"db_form.php?table=host&amp;detail=low\">Hosts</a></li>
        <li><a href=\"db_form.php?table=user\">Users</a> (<a href=\"list_new_users.php\">recently registered</a>)</li>
        <li><a href=\"db_form.php?table=team\">Teams</a></li>
        <li><a href=\"db_action.php?table=app\">Applications</a></li>
        <li><a href=\"db_form.php?table=app_version\">Application versions</a></li>
        <li><a href=\"db_action.php?table=platform\">Platforms</a></li>
        <li><a href=dbinfo.php>DB row counts and disk usage</a>
        <li><a href=sample_table_stats.php>DB table details</a>
        <li><a href=\"show_log.php?f=mysql*.log&amp;l=-20\">Tail MySQL logs</a>
    </ul>


    </td>
    <td><b>Computing</b>
    <ul>
        <li><a href=\"manage_apps.php\">Manage applications</a></li>
        <li><a href=\"manage_app_versions.php\">Manage application versions</a></li>
        <li><a href=\"manage_consent_types.php\">Manage consent types</a></li>
        <li> Manage jobs
        <ul>
            <li><a href=\"cancel_wu_form.php\">Cancel jobs by ID</a>
            <li><a href=\"cancel_workunits.php\">Cancel jobs by SQL clause</a>
            <li><a href=transition_all.php>Transition jobs</a>
              <p class=\"text-muted\">(this can 'unstick' old jobs)</p>
            <li><a href=\"revalidate.php\">Re-validate jobs</a>
            <li><a href=assign.php>Assigned jobs</a>
        </ul>
        <li><a href=\"job_times.php\">FLOP count statistics</a>
        <li><a href=\"$stripchart_cgi_url/stripchart.cgi\">Stripcharts</a>
        <li><a href=\"show_log.php\">Show/Grep logs</a>
        <li>
            <form method=\"get\" action=\"clear_host.php\">
            <input class=\"btn btn-default\" type=\"submit\" value=\"Clear RPC seqno\">
            host ID:
            <input type=\"text\" size=\"5\" name=\"hostid\">
            </form>
    </ul>

    </td>
    <td><b>User management</b>
    <ul>
        <li><a href=".url_base()."/forum_index.php>Post news item</a></li>
        <li><a href=\"profile_screen_form.php\">Screen user profiles </a></li>
        <li><a href=\"badge_admin.php\">Badges</a></li>
        <li><a href=\"manage_special_users.php\">User privileges</a></li>
        <li><a href=".url_base()."/manage_project.php>User job submission privileges</a></li>
        <li><a href=\"mass_email.php\">Send mass email to a selected set of users</a></li>
        <li><a href=\"check_account_ownership_keys.php\">Check generated account ownership keys</a></li>
        <li><form action=\"manage_user.php\">
            <input class=\"btn btn-default\" type=\"submit\" value=\"Manage user\">
            ID: <input name=\"userid\">
            </form>
        </li>
    </ul>
    </td>
    </tr>
    </table>
";

// Result Summaries:

$show_deprecated = get_str("show_deprecated", true);
$show_only = array("all"); // Add all appids you want to display, or "all"
$apps = BoincApp::enum("");
foreach ($apps as $app) {
    if (in_array($app->id, $show_only)
       || ( in_array("all", $show_only)
          && (!$app->deprecated || $show_deprecated)
    )) {

        echo "
            <b>Results for <tt>$app->name</tt>:</b>
            <ul>
";
        for ($i=0; $i<2; $i++) {
            if ($i) {
                $secs = 7*86400;
                $period = "&nbsp;&nbsp;&nbsp;7 days";
            } else {
                $secs = 86400;
                $period = "24 hours";
            }
            echo "
                <li> Past $period:
                <a href=\"result_summary.php?appid=$app->id&amp;nsecs=$secs\">
                    summary
                </a> |
                <a href=\"pass_percentage_by_platform.php?appid=$app->id&amp;nsecs=$secs\">
                    summary per app version
                </a> |
                <a href=\"failure_result_summary_by_host.php?appid=$app->id&amp;nsecs=$secs\">
                    failures broken down by (app version, host)
                </a> |
                <a href=\"failure_result_summary_by_platform.php?appid=$app->id&amp;nsecs=$secs\">
                    failures broken down by (app version, error)
                </a>
";
        }
        echo " </ul> ";
    }
}

if ($show_deprecated) {
    echo "<a href=\"index.php?show_deprecated=0\">Hide deprecated applications</a>";
} else {
    echo "<a href=\"index.php?show_deprecated=1\">Show deprecated applications</a>";
}

echo "<h3>Periodic tasks</h3>
The following scripts should be run as periodic tasks, not via this web page
(see <a href=\"https://github.com/BOINC/boinc/wiki/ProjectTasks\">https://github.com/BOINC/boinc/wiki/ProjectTasks</a>):
<pre>
    update_forum_activities.php, update_profile_pages.php, update_uotd.php, delete_expired_tokens.php, delete_expired_users_and_hosts.php
</pre>

<h3>Repair tasks</h3>
The following scripts do one-time repair operations.
Run them manually on the command line as needed
(i.e. <tt>php scriptname.php</tt>):
<pre>forum_repair.php, team_repair.php, repair_validator_problem.php</pre>

<h3>Cleanup tasks</h3>
You can run the following as a periodic task, on the command line,
or by clicking here:
    <ul>
    <li> <a href=remove_zombie_hosts.php>remove_zombie_hosts.php</a> Remove zombie host records
    </ul>
";

admin_page_tail();

?>
