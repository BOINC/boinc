<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

function svn_revision($path) {
    $out = array();
    $cmd = "svn info http://boinc.berkeley.edu/svn/$path";
    if (defined("SVN_CONFIG_DIRECTORY")) {
        $cmd .= " --config-dir ". SVN_CONFIG_DIRECTORY;
    }
    exec($cmd, $out);
    foreach ($out as $line) {
        $x = strstr($line, "Last Changed Rev: ");
        if ($x) {
            $y = substr($x, strlen("Last Changed Rev: "));
            return (int) $y;
        }
    }
    return null;
}

$config = get_config();
$cgi_url = parse_config($config, "<cgi_url>");
$stripchart_cgi_url = parse_config($config, "<stripchart_cgi_url>");

db_init();

$title = "Project Management";
admin_page_head($title);

// Notification area
echo "<ul>\n";

echo "<li>";
if (file_exists("../../local.revision")) {
    $local_rev = file_get_contents("../../local.revision");
}
if ($local_rev) {
    echo "Using BOINC SVN revision: ".$local_rev."; ";
}

if (0
//if (file_exists("../cache/remote.revision")
//    && (time() < filemtime("../cache/remote.revision")+(24*60*60))
) {
    $remote_rev = file_get_contents("../cache/remote.revision");
} else {
    $remote_rev = svn_revision("branches/server_stable");
}

if ($remote_rev) {
    echo "BOINC server_stable SVN revision: $remote_rev";
} else {
    echo "Can't get BOINC server_stable SVN revision";
}

if (!file_exists(".htaccess")) {
    echo "<li><span style=\"color: #ff0000\">The Project Management directory is not
        protected from public access by a .htaccess file.</span></li>\n";
}

if (!defined("SYS_ADMIN_EMAIL")) {
    echo "<li><span style=\"color: #ff0000\">The defined constant SYS_ADMIN_EMAIL
        has not been set. Please edit <tt>project/project.inc</tt> and set this
        to an address which can be used to contact the project administrators.
        </span></li>\n";
}

if (parse_bool($config, "disable_account_creation")) {
    echo "<li><span style=\"color: #ff9900\">Account creation is disabled.</span></li>\n";
}

if (defined("INVITE_CODES")) {
    echo "<li><span style=\"color: #ff9900\">Account creation is restricted by the use of
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
    echo "<li><span style=\"color: ".$color."\">There are ".$uotd_candidates." remaining
        candidates for User of the Day.</span></li>\n";
}

echo "</ul>\n";

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
        <li><a href=\"show_log.php?f=mysql*.log&amp;l=-20\">Tail MySQL logs</a>
    </ul>
    

    </td> 
    <td><b>Computing</b>
    <ul>
        <li><a href=\"manage_apps.php\">Manage applications</a></li>
        <li><a href=\"manage_app_versions.php\">Manage application versions</a></li>
        <li> Manage jobs
        <ul>
            <li><a href=\"cancel_wu_form.php\">Cancel jobs</a></li>
            <li><a href=transition_all.php>Transition jobs</a>
              <br><span class=note>(this can 'unstick' old jobs)</span>
            <li><a href=\"revalidate.php\">Re-validate jobs</a></li>

        </ul>
        <li><a href=\"job_times.php\">FLOP count statistics</a>
        <li><a href=\"$stripchart_cgi_url/stripchart.cgi\">Stripcharts</a>
        <li><a href=\"show_log.php\">Show/Grep logs</a>
        <li>
            <form method=\"get\" action=\"clear_host.php\">
            <input type=\"submit\" value=\"Clear RPC seqno\">
            host ID: 
            <input type=\"text\" size=\"5\" name=\"hostid\">
            </form>
    </ul>
    
    </td> 
    <td><b>User management</b>
    <ul>
        <li><a href=\"profile_screen_form.php\">Screen user profiles </a></li>
        <li><a href=\"manage_special_users.php\">User privileges</a></li>
        <li><a href=".URL_BASE."/manage_project.php>User job submission privileges</a></li>
        <li><a href=\"mass_email.php\">Send mass email to a selected set of users</a></li>
        <li><a href=\"problem_host.php\">Email user with misconfigured host</a></li>
        <li><form action=\"manage_user.php\">
            <input type=\"submit\" value=\"Manage user\">
            ID: <input name=\"userid\">
            </form>
        </li>
        </li>
    </ul>
    </td>
    </tr>
    </table>
";

// Result Summaries:

$show_deprecated = get_str("show_deprecated", true);
$show_only = array("all"); // Add all appids you want to display, or "all"
$result = mysql_query("select id, name, deprecated from app");
while ($app = mysql_fetch_object($result)) {
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
mysql_free_result($result);

if ($show_deprecated) {
    echo "<a href=\"index.php?show_deprecated=0\">Hide deprecated applications</a>";
} else {
    echo "<a href=\"index.php?show_deprecated=1\">Show deprecated applications</a>";
}

// Periodic tasks

echo "<h3>Periodic or special tasks</h3>
    <ul>
    <li> The following scripts should be run as periodic tasks,
        not via this web page
        (see <a href=\"http://boinc.berkeley.edu/trac/wiki/ProjectTasks\">http://boinc.berkeley.edu/trac/wiki/ProjectTasks</a>):
        <pre> update_forum_activities.php, update_profile_pages.php, update_uotd.php</pre>
    <li> The following scripts can be run manually on the command line
        as needed (i.e. <tt>php scriptname.php</tt>):
        <pre>forum_repair.php, team_repair.php, repair_validator_problem.php</pre>
   </ul>
    ";

admin_page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
