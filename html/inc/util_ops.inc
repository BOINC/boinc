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
require_once("../inc/util.inc");
require_once("../project/project.inc");

// TODO: get rid of the following.  use style sheets
define("TD", "<td bgcolor=\"#708090\">");
define("TD2", "<td colspan=\"2\" bgcolor=\"#708090\">");
define("TD3", "<td colspan=\"3\" bgcolor=\"#708090\">");
define("TABLE", "<table cellpadding=\"8\" border=\"1\" width=\"100%\">");
define("TABLE2", "<table width=\"580\">");

function admin_page_head($title) {
    $stylesheet = URL_BASE.STYLESHEET;

    echo "<html><head><title>$title</title>
        <link rel=stylesheet type=\"text/css\" href=\"".URL_BASE."main.css\" media=\"all\" />
        <link rel=\"stylesheet\" type=\"text/css\" href=\"$stylesheet\">
        </head>
        <body bgcolor=\"#ffffff\">
        <h2>".PROJECT.": $title</h2>
    ";
}

function admin_page_tail() {
    echo "<br><hr noshade size=1><center>
        <a href=\"index.php\"> Back to Main admin page </a> |
    ";
    $user = get_logged_in_user_ops();
    if ($user) {
        echo "logged in as $user->name. <a href=logout.php>Log out</a>";
    } else {
        echo "<a href=login_form.php>Log in</a>";
    }

    echo "
        </center>
        </body></html>
    ";
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
    echo "<tr><td valign=\"top\" align=\"right\">$x</td>\n<td>$y</td>\n</tr>\n";
}

function c_row2($color, $x, $y) {
    echo "<tr bgcolor=\"$color\"><td align=\"right\">$x</td><td>$y</td></tr>\n";
}

function show_profile_link_ops($user) {
    if ($user->has_profile) { 
        row2("Profile",
            "<a href=\"".URL_BASE."view_profile.php?userid=$user->id\">View</a>"
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
    print "Database username (if not current user): ";
    $user = rtrim(fgets($in, 80));
    print "Database password (if any): ";
    $pass = rtrim(fgets($in, 80));

    $retval = mysql_pconnect($host, $user, $pass);
    if (!$retval) {
        die("Can't connect to DB\n");
    }
    if (!mysql_select_db($db_name)) {
        die("Can't select DB\n");
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
    row2("", "<input type=submit value=OK>");
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
                <a href=http://boinc.berkeley.edu/trac/wiki/HtmlOps>See how here</a>"
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

function test_mysql_query($msg) {
    echo "mysql_query($msg)<br/>";
    return 1;
}

// for purposes of testing and seeing queries,
// replace the two instances of mysql_query() below with test_mysql_query().
//
function cancel_wu($wuid1, $wuid2) {
    $command1="update result set server_state=5, outcome=5 where server_state=2 and $wuid1<=workunitid and workunitid<=$wuid2";
    $command2="update workunit set error_mask=error_mask|16 where $wuid1<=id and id<=$wuid2";

    if (!mysql_query($command1)) {
        echo "MySQL command $command1 failed:<br/>unable to cancel unsent results.<br/>";
        return 1;
    } else if (!mysql_query($command2)) {
        echo "MySQL command $command2 failed:<br/>unable to cancel workunits.<br/>";
        return 2;
    }

    // trigger the transitioner (it will set file_delete_state)

    $now = time();
    $query="update workunit set transition_time=$now where $wuid1<=id and id<=$wuid2";
    mysql_query($query);

    return 0;
}

if (isset($cli_only)) {
    if (array_key_exists("SERVER_PORT", $_SERVER)) {
        die("This script is intended to be run from the command line,
            not from the web server."
        );
    }
}

if (!isset($skip_auth_ops) && array_key_exists("SERVER_PORT", $_SERVER)) {
    auth_ops();
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
