<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// utility functions for admin web pages

require_once("../inc/db_ops.inc");
require_once("../inc/util.inc");
require_once("../project/project.inc");

display_errors();

function admin_page_head($title) {
    echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">";
    echo sprintf('<html><head><title>%s</title>
        <meta http-equiv="content-type" content="text/html;charset=utf-8" />
        <link type="text/css" rel="stylesheet" href="%s/bootstrap.min.css" media="all">
        <link type="text/css" rel="stylesheet" href="%s/custom.css" media="all">
        </head>
        <body>
        <div class="container-fluid">
        <h2>%s: %s</h2>
        ',
        $title,
        secure_url_base(),
        secure_url_base(),
        PROJECT,
        $title
    );
    show_login_info();
    echo "<p>";
}

function admin_page_tail() {
    echo sprintf('
        <hr><center><a href=index.php>Main page</a></center>
        <script src="%s/jquery.min.js"></script>
        <script src="%s/bootstrap.min.js"></script>
        </div>
        </body>
        </html>
        ',
        secure_url_base(),
        secure_url_base()
    );
}

// TODO: get rid of all the following

function print_checkbox($text,$name,$checked) {
    echo "<input type=\"checkbox\" name=\"$name\""
        . (strlen($checked) ? " checked=\"checked\"" : "") . ">"
        . "$text\n"
        . "<p>\n";
}

function print_radio_button($text,$name,$value,$checked) {
    echo "<input type=\"radio\" name=\"$name\" value=\"$value\""
        . (strlen($checked) ? " checked=\"checked\"" : "") . ">"
        . "$text\n"
        . "<br>\n";
}

function print_text_field($text,$name,$value) {
    echo "$text <input type=\"text\" size=\"10\" name=\"$name\" value=\"$value\">\n"
         . "<p>\n";
}

function row($x, $y) {
    echo "<tr><td width=30% valign=\"top\" align=\"right\">$x &nbsp;&nbsp; </td>\n<td>$y</td>\n</tr>\n";
}

function c_row2($color, $x, $y) {
    echo "<tr bgcolor=\"$color\"><td align=\"right\">$x</td><td>$y</td></tr>\n";
}

function show_profile_link_ops($user) {
    if ($user->has_profile) {
        row2("Profile",
            "<a href=\"".url_base()."view_profile.php?userid=$user->id\">View</a>"
        );
    }
}

// initialize database connection with username & password from
// command line instead of config.xml
//
function db_init_cli() {
    $config = get_config();
    $db_name = parse_config($config, "<db_name>");
    $host = parse_config($config, "<db_host>");
    if ($host == null) {
        $host = "localhost";
    }
    $in = fopen("php://stdin","r");
    print "Database username (default: owner of mysqld process): ";
    $user = rtrim(fgets($in, 80));
    print "Database password (if any): ";
    $pass = rtrim(fgets($in, 80));

    $retval = _mysql_connect($host, $user, $pass, $db_name);
    if (!$retval) {
        die("Can't connect to DB\n");
    }
}

function print_login_form_ops($next_url='') {
    if ($next_url == '') $next_url = $_SERVER['REQUEST_URI'];
    start_table();
    echo "
        <form method=post action=login_action.php>
        <input type=hidden name=next_url value=$next_url>
    ";
    row2("Email", "<input name=email_addr size=40>");
    row2("Password", "<input type=password name=passwd size=40>");
    row2(tra("Stay logged in on this computer"), '<input type="checkbox" name="stay_logged_in" checked>');
    row2("", "<input class=\"btn btn-primary\" type=submit value=OK>");
    end_table();
}

function get_logged_in_user_ops() {
    global $g_logged_in_user;
    if ($g_logged_in_user) return $g_logged_in_user;
    $authenticator = null;
    if (isset($_COOKIE['auth'])) $authenticator = $_COOKIE['auth'];

    $authenticator = BoincDb::escape_string($authenticator);
    if ($authenticator) {
        $g_logged_in_user = BoincUser::lookup("authenticator='$authenticator'");
    }
    return $g_logged_in_user;
}

////////// functions for access control of admin web pages /////////////

// allow access only if logged in as user in a given set
//
function auth_ops_userid($admin_user_ids) {
    $user = get_logged_in_user_ops();
    if (!$user) {
        admin_page_head("Log in");
        echo "You must log in to performance admin functions.<p>\n";
        print_login_form_ops();
        admin_page_tail();
        exit;
    } else if (!in_array($user->id, $admin_user_ids)) {
        admin_page_head("Log in");
        echo "
            You must be logged in as an admin to perform admin functions.
            <p>
            <a href=logout.php>Log out</a>
        ";
        admin_page_tail();
        exit;
    }
}

// allow access only to users with ADMIN/DEV flags in forum prefs.
// If you use this, make sure you know who has these privileges
//
function auth_ops_privilege() {
    $user = get_logged_in_user_ops();
    if (!$user) {
        admin_page_head("Log in");
        echo "You must log in to performance admin functions.<p>\n";
        print_login_form_ops();
        admin_page_tail();
        exit;
    }
    BoincForumPrefs::lookup($user);
    if ($user->prefs->privilege(S_ADMIN) || $user->prefs->privilege(S_DEV)) {
        return;
    }
    error_page("Access denied");
}

// if project hasn't specified a policy in project.inc,
// and no .htaccess, don't allow access
//
if (!function_exists('auth_ops')) {
    function auth_ops() {
        if (!file_exists(".htaccess")) {
            error_page("
                You must protect the admin interface
                with either a .htaccess file or an auto_ops() function.
                <p>
                <a href=https://github.com/BOINC/boinc/wiki/HtmlOps>See how here</a>"
            );
        }
    }
}

function admin_error_page($msg) {
    admin_page_head("Unable to handle request");
    echo $msg;
    admin_page_tail();
    exit;
}

// given a list of app versions,
// return a list of the current, non-deprecated ones
//
function current_versions($avs) {
    foreach($avs as $av) {
        foreach ($avs as $av2) {
            if ($av->id == $av2->id) continue;
            if ($av->platformid == $av2->platformid && $av->plan_class == $av2->plan_class && $av->version_num > $av2->version_num) {
                $av2->deprecated = 1;
            }
        }
    }
    $x = array();
    foreach($avs as $av) {
        if (!$av->deprecated) $x[] = $av;
    }
    return $x;
}

// cancel WUs with IDs in a given range.
// This means:
//
// - for any results w/ server state UNSENT, set server state to OVER
// - set the CANCELLED bit in workunit.error_mask
//
function cancel_wus($wuid1, $wuid2) {
    $retval = BoincResult::update_aux(
        sprintf(
            'server_state=%d, outcome=%d where server_state=%d and workunitid>=%d and workunitid<=%d',
            RESULT_SERVER_STATE_OVER,
            RESULT_OUTCOME_DIDNT_NEED,
            RESULT_SERVER_STATE_UNSENT,
            $wuid1, $wuid2
        )
    );
    if (!$retval) {
        error_page("Result update failed");
    }

    // mark WUs as cancelled and trigger the transitioner
    //
    $retval = BoincWorkunit::update_aux(
        sprintf(
            'error_mask=error_mask|%d, transition_time=%d where id>=%d and id<=%d',
            WU_ERROR_CANCELLED,
            time(),
            $wuid1, $wuid2
        )
    );
    if (!$retval) {
        error_page("Workunit update failed");
    }
    return 0;
}

// like above, but if a workunit has a result that's already sent,
// don't cancel the workunit
//
function cancel_wus_if_unsent($id1, $id2) {
    $wus = BoincWorkunit::enum("id >= $id1 and id <= $id2");
    foreach ($wus as $wu) {
        $results = BoincResult::enum(
            sprintf(
                'workunitid=%d and server_state > %d',
                $wu->id, RESULT_SERVER_STATE_UNSENT
            )
        );
        if (count($results)) continue;
        $retval = BoincResult::update_aux(
            sprintf(
                'server_state=%d, outcome=%d where workunitid=%d',
                RESULT_SERVER_STATE_OVER,
                RESULT_OUTCOME_DIDNT_NEED,
                $wu->id
            )
        );
        if (!$retval) {
            error_page("result update failed");
        }
        if (!$wu->update("error_mask=error_mask|16")) {
            error_page("WU update failed");
        }
    }
    return 0;
}

function app_version_desc($avid) {
    switch ($avid) {
    case ANON_PLATFORM_UNKNOWN:
        return "Anonymous platform: unknown type";
    case ANON_PLATFORM_CPU:
        return "Anonymous platform: CPU";
    case ANON_PLATFORM_NVIDIA:
        return "Anonymous platform: NVIDIA GPU";
    case ANON_PLATFORM_ATI:
        return "Anonymous platform: AMD GPU";
    case ANON_PLATFORM_INTEL_GPU:
        return "Anonymous platform: Intel GPU";
    case ANON_PLATFORM_APPLE_GPU:
        return "Anonymous platform: Apple GPU";
    }
    if ($avid <= 0) {
        return "unknown: $avid";
    }
    $av = BoincAppVersion::lookup_id($avid);
    if ($av) {
        $p = BoincPlatform::lookup_id($av->platformid);
        if ($p) {
            return sprintf("%.2f", $av->version_num/100)." $p->name [$av->plan_class]";
        } else {
            return sprintf("%.2f", $av->version_num/100)." MISSING PLATFORM $av->platformid [$av->plan_class]";
        }
    } else {
        return "App version missing ($avid)";
    }
}

////// badge-related stuff

function get_badge($name, $title, $image_url) {
    $name = BoincDb::escape_string($name);
    $title = BoincDb::escape_string($title);
    $image_url = BoincDb::escape_string($image_url);
    $b = BoincBadge::lookup("name='$name'");
    if ($b) return $b;
    $now = time();
    $id = BoincBadge::insert("(create_time, type, name, title, description, image_url, level, tags, sql_rule) values ($now, 0, '$name', '$title', '', 'img/$image_url', '', '', '')");
    $b = BoincBadge::lookup_id($id);
    if ($b) return $b;
    die("can't create badge $name\n");
}

function assign_badge($is_user, $item, $badge) {
    $now = time();
    if ($is_user) {
        $bbu = BoincBadgeUser::lookup("user_id=$item->id and badge_id=$badge->id");
        if ($bbu) {
            $bbu->update("reassign_time=$now where user_id=$item->id and badge_id=$badge->id");
        } else {
            BoincBadgeUser::insert("(create_time, user_id, badge_id, reassign_time) values ($now, $item->id, $badge->id, $now)");
        }
    } else {
        $bbt = BoincBadgeTeam::lookup("team_id=$item->id and badge_id=$badge->id");
        if ($bbt) {
            $bbt->update("reassign_time=$now where team_id=$item->id and badge_id=$badge->id");
        } else {
            BoincBadgeTeam::insert("(create_time, team_id, badge_id, reassign_time) values ($now, $item->id, $badge->id, $now)");
        }
    }
}

// unassign all badges except the given one
//
function unassign_badges($is_user, $item, $badges, $k) {
    $list = null;
    for ($i=0; $i<count($badges); $i++) {
        if ($i == $k) continue;
        $badge = $badges[$i];
        if ($list) {
            $list .= ",$badge->id";
        } else {
            $list = "$badge->id";
        }
    }
    if ($is_user) {
        BoincBadgeUser::delete("user_id=$item->id and badge_id in ($list)");
    } else {
        BoincBadgeTeam::delete("team_id=$item->id and badge_id in ($list)");
    }
}

////// end badge-related stuff

function running_from_web_server() {
    return array_key_exists("SERVER_PORT", $_SERVER);
}

if (isset($cli_only)) {
    if (running_from_web_server()) {
        die("This script is intended to be run from the command line,
            not from the web server."
        );
    }
}

if (!isset($skip_auth_ops) && array_key_exists("SERVER_PORT", $_SERVER)) {
    auth_ops();
}

// returns true when this is a readonly ops section
// currently a dummy because this needs to be ported from Einstein@home
//
function in_rops() {
    return false;
}

function cancel_wus_where($clause) {
    $q1 = "CREATE TEMPORARY TABLE tmp SELECT id FROM workunit WHERE $clause;";
    $q2 = sprintf(
        'UPDATE result r INNER JOIN tmp t on r.workunitid=t.id SET outcome=%d WHERE server_state=%d;',
        RESULT_OUTCOME_DIDNT_NEED, RESULT_SERVER_STATE_UNSENT
    );
    $q3 = "UPDATE workunit w INNER JOIN tmp t on w.id=t.id SET error_mask=error_mask|16, transition_time=0;";
    $q4 = "DROP TABLE tmp;";

    $db = BoincDb::get();

    if (!$db->do_query($q1)) {
        echo "MySQL command '$q1' failed:<br/>unable to create temporary WU id table.<br>\n";
        return 1;
    } else if (!$db->do_query($q2)) {
        echo "MySQL command '$q2' failed:<br/>unable to cancel unsent results.<br>\n";
        $db->do_query($q4);
        return 2;
    } else if (!$db->do_query($q3)) {
      echo "MySQL command '$q3' failed:<br/>unable to cancel workunits and trigger transitioner.<br>\n";
        $db->do_query($q4);
        return 3;
    }
    $db->do_query($q4);
    echo "Successfully canceled WUs WHERE '$clause'<br>\n";
    return 0;
}

?>
