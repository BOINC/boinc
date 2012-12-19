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

require_once("../inc/util_basic.inc");
require_once("../project/project.inc");
require_once("../inc/countries.inc");
require_once("../inc/db.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/translation.inc");
require_once("../inc/profile.inc");

// don't allow /... at the end of URL

if (array_key_exists("PATH_INFO", $_SERVER)) {
    die("bad URL");
}

ini_set("memory_limit", "256M");

if (!defined('SECURE_URL_BASE')) {
    define('SECURE_URL_BASE', URL_BASE);
}

// define TIMEZONE in project.inc
//
if (defined('TIMEZONE')) {
    date_default_timezone_set(TIMEZONE);
} else {
    date_default_timezone_set('UTC');
}

$generating_xml = false;
$caching = false;
$did_page_head = false;

function send_cookie($name, $value, $permanent, $ops=false) {
    // the following allows independent login for projects on the same server
    //
    $master_url = parse_config(get_config(), "<master_url>");
    $url = parse_url($master_url);
    $path = $url['path'];
    if ($ops) {
        $path = substr($path, 0, -1);
        $path .= "_ops/";
    }
    $expire = $permanent?time()+3600*24*365:0;
    setcookie($name, $value, $expire, $path);
}

function clear_cookie($name, $ops=false) {
    $master_url = parse_config(get_config(), "<master_url>");
    $url = parse_url($master_url);
    $path = $url['path'];
    if ($ops) {
        $path = substr($path, 0, -1);
        $path .= "_ops/";
    }
    setcookie($name, '', time()-3600, $path);
}

function get_user_from_id($id) {
    if ($id) return lookup_user_id($id);
    return NULL;
}

$g_logged_in_user = null;
$got_logged_in_user = false;

function get_logged_in_user($must_be_logged_in=true) {
    global $g_logged_in_user, $got_logged_in_user;
    if ($got_logged_in_user) return $g_logged_in_user;

    check_web_stopped();

    $authenticator = null;
    if (isset($_COOKIE['auth'])) $authenticator = $_COOKIE['auth'];

    $authenticator = BoincDb::escape_string($authenticator);
    if ($authenticator) {
        $g_logged_in_user = BoincUser::lookup("authenticator='$authenticator'");
    }
    if ($must_be_logged_in && !$g_logged_in_user) {
        $next_url = '';
        if (array_key_exists('REQUEST_URI', $_SERVER)) {
            $next_url = $_SERVER['REQUEST_URI'];
            $n = strrpos($next_url, "/");
            if ($n) {
                $next_url = substr($next_url, $n+1);
            }
        }
        $next_url = urlencode($next_url);
        Header("Location: login_form.php?next_url=$next_url");
        exit;
    }
    $got_logged_in_user = true;
    return $g_logged_in_user;
}

function show_login_info($prefix="") {
    $user = get_logged_in_user(false);
    echo "
        <table width=\"100%\" cellpadding=0 cellspacing=0>
        <tr><td align=right>
    ";
    if ($user) {
        $url_tokens = url_tokens($user->authenticator);
        echo "<nobr>$user->name &middot; <a href=".$prefix."logout.php?$url_tokens>".tra("log out")."</a></nobr>";
    } else {
        echo "<a href=".$prefix."login_form.php>".tra("log in")."</a>";
    }
    echo "
        </td>
        </tr>
        </table>
    ";
}

$cache_control_extra="";

// Page_head() is overridable so that projects that want to integrate BOINC
// with an existing web framework can more easily do so.
// To do so, define page_head() in the project include file.
//
if (!function_exists("page_head")){
function page_head(
    $title, $java_onload=null, $title_plain=null, $prefix="", $head_extra=null
) {
    global $caching, $cache_control_extra, $did_page_head;

    $did_page_head = true;
    $stylesheet = URL_BASE.STYLESHEET;
    $rssname = PROJECT . " RSS 2.0";
    $rsslink = URL_BASE."rss_main.php";
    if (defined('STYLESHEET2')) {
        $stylesheet2 = URL_BASE.STYLESHEET2;
    } else {
        $stylesheet2 = null;
    }


    header("Content-type: text/html; charset=utf-8");
    if (!$caching) {
        header ("Expires: Mon, 26 Jul 1997 05:00:00 UTC");    // Date in the past
        header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " UTC"); // always modified
        header ("Cache-Control: $cache_control_extra no-cache, must-revalidate, post-check=0, pre-check=0");  // HTTP/1.1
        header ("Pragma: no-cache");                          // HTTP/1.0
    }

    echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">";
    
    echo "<html><head>\n";
    if ($head_extra) {
        echo "\n$head_extra\n";
    }
    if (!$title_plain) {
        echo "<title>".sanitize_tags($title)."</title>\n";
    } else {
        echo "<title>".sanitize_tags($title_plain)."</title>\n";
    }
    echo "<link rel=stylesheet type=\"text/css\" href=\"".URL_BASE."main.css\" media=\"all\">
        <link rel=stylesheet type=\"text/css\" href=\"$stylesheet\">
    ";
    if ($stylesheet2) {
        echo "<link rel=stylesheet type=\"text/css\" href=\"$stylesheet2\">\n";
    }
    echo "
        <link rel=alternate type=\"application/rss+xml\" title=\"$rssname\" href=\"$rsslink\">
        </head>
    ";
    if ($java_onload){
        echo "<body onload=\"".$java_onload."\">";
    } else {
        echo "<body>";
    }
    display_cvs_versions();

    project_banner($title, $prefix);
    switch($title) {    //kludge
    case tra("Log in"):
    case tra("Create an account"):
    case tra("Server status page"):
        break;
    default:
        show_login_info($prefix);
    }

}
}

// the following must be included in header
// for Recaptcha to work with some IE browsers
//
define('IE_COMPAT_MODE', '<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE8" >');

function page_tail_aux($show_return, $show_date, $prefix="") {
    project_footer($show_return, $show_date, $prefix);
    echo "</body>
        </html>
    ";
}
function page_tail_main($show_date=false) {
    page_tail_aux(false, $show_date);
}

// See the comments for page_head()
//
if (!function_exists("page_tail")){
function page_tail($show_date=false, $prefix="") {
    page_tail_aux(true, $show_date, $prefix);
}
}

function display_cvs_versions(){
    global $cvs_version_tracker;
    echo "\n<!-- SVN VERSIONS -->\n";
    for ($i=0;$i<sizeof($cvs_version_tracker);$i++) {
        echo "<!-- ".$cvs_version_tracker[$i]." -->\n";
    }
}

function db_error_page() {
    page_head("Database error");
    echo tra("A database error occurred while handling your request; please try again later.");
    page_tail();
}

function error_page($msg) {
    global $generating_xml;
    if ($generating_xml) {
        xml_error(-1, $msg);
    }
    page_head(tra("Unable to handle request"));
    echo $msg;
    page_tail();
    exit();
}

// takes argument in second and returns a human formatted time string
// in the form D days + h Hours + m Min + s sec.

function time_diff($x) {
    $days    = (int)($x/86400);
    $hours   = (int)(($x-$days*86400)/3600);
    $minutes = (int)(($x-$days*86400-$hours*3600)/60);
    $seconds = (int)($x % 60);

    $datestring = "";
    if ($days) {
        $datestring .= "$days ".tra("days")." ";
    }
    if ($hours || strlen($datestring)) {
        $datestring .= "$hours ".tra("hours")." ";
    }
    if ($minutes || strlen($datestring)) {
        $datestring .= "$minutes ".tra("min")." ";
    }
    if ($seconds) {
        $datestring .= "$seconds ".tra("sec")." ";
    }

    return $datestring;
}


function date_str($x) {
    if ($x == 0) return "---";
    return gmdate('j M Y', $x);
}

function time_str($x) {
    if ($x == 0) return "---";
    return gmdate('j M Y, G:i:s', $x) . " UTC";
}

function local_time_str($x) {
    if ($x == 0) return "---";
    return date('j M Y, H:i T', $x);
}

function pretty_time_str($x) {
    return time_str($x);
}

function start_table($extra="width=\"100%\"") {
    echo "<table class=bordered $extra>";
}

function start_table_noborder($width="100%") {
    echo "<table cellpadding=5 width=\"$width\">";
}

function end_table() {
    echo "</table>\n";
}

// Table header row with unlimited number of columns

function table_header() {
    echo "<tr>\n";
    for ($i = 0; $i < func_num_args(); $i++) {
        if (is_array(func_get_arg($i))) {
            $col = func_get_arg($i);
            echo "<th ".$col[1].">".$col[0]."</th>\n";
        } else {
            echo "<th>".func_get_arg($i)."</th>\n";
        }
    }
    echo "</tr>\n";
}

// Table row with unlimited number of columns

function table_row() {
    echo "<tr>\n";
    for ($i = 0; $i < func_num_args(); $i++) {
        if (is_array(func_get_arg($i))) {
            $col = func_get_arg($i);
            echo "<td ".$col[1].">".$col[0]."</td>\n";
        } else {
            echo "<td>".func_get_arg($i)."</td>\n";
        }
    }
    echo "</tr>\n";
}

function row1($x, $ncols=2, $class="heading") {
    echo "<tr><td class=\"$class\" colspan=\"$ncols\">$x</td></tr>\n";
}

function row2($x, $y, $show_error=false) {
    if ($x=="") $x="<br>";
    if ($y==="") $y="<br>";
    if ($show_error) {
        $class1 = 'fieldname_error';
        $class2 = 'fieldvalue_error';
    } else {
        $class1 = 'fieldname';
        $class2 = 'fieldvalue';
    }
    echo "<tr><td width=\"40%\" class=$class1>$x</td><td class=$class2>$y</td></tr>\n";
}

function row2_init($x, $y) {
    echo "<tr><td class=fieldname width=\"40%\">$x</td><td class=fieldvalue>$y\n";
}

function row2_plain($x, $y) {
    echo "<tr><td>$x</td><td>$y</td></tr>\n";
}

function row3($x, $y, $z) {
    echo "<tr><td width=\"30%\" align=\"right\">$x</td><td>$y</td><td>$z</td></tr>\n";
}

function row4($xx, $xy, $yx, $yy) {
    echo "<tr><td width=\"25%\">$xx</td><td width=\"25%\">$xy</td>"
        . "<td width=\"25%\">$yx</td><td width=\"%25\">$yy</td></tr>
    ";
}

function rowify($string) {
    echo "<tr><td>$string</td></tr>";
}

function row_array($x) {
    echo "<tr>";
    foreach ($x as $h) {
        echo "<td>$h</td>";
    }
    echo "</tr>\n";
}

function row_heading_array($x) {
    echo "<tr>";
    foreach ($x as $h) {
        echo "<th>$h</th>";
    }
    echo "</tr>\n";
}

function random_string() {
    return md5(uniqid(rand(), true));
}

function url_tokens($auth) {
    $now = time();
    $ttok = md5((string)$now.$auth);
    return "&amp;tnow=$now&amp;ttok=$ttok";
}

function form_tokens($auth) {
    $now = time();
    $ttok = md5((string)$now.$auth);
    return "<input type=\"hidden\" name=\"tnow\" value=\"$now\">
        <input type=\"hidden\" name=\"ttok\" value=\"$ttok\">
    ";
}

function valid_tokens($auth) {
    $tnow = get_str('tnow', true);
    $ttok = get_str('ttok', true);
    if (!$tnow) {
        $tnow = $_POST['tnow'];
    }
    if (!$ttok) {
        $ttok = $_POST['ttok'];
    }
    if (!$tnow) return false;
    if (!$ttok) return false;
    $t = md5((string)$tnow.$auth);
    if ($t != $ttok) return false;
    if (time() > $tnow + 86400) return false;
    return true;
}

function check_tokens($auth) {
    if (valid_tokens($auth)) return;
    error_page(
        tra("Link has timed out. Please click Back, refresh the page, and try again.")
    );
}

function no_computing() {
    return parse_bool(get_config(), "no_computing");
}

// Generates a legal filename from a parameter string.

function get_legal_filename($name) {
    return strtr($name, array(','=>'', ' '=>'_'));
}

// Returns a string containing as many words
// (being collections of characters separated by the character $delimiter)
// as possible such that the total string length is <= $chars characters long.
// If $ellipsis is true, then an ellipsis is added to any sentence which
// is cut short.

function sub_sentence($sentence, $delimiter, $max_chars, $ellipsis=false) {
    $words = explode($delimiter, $sentence);
    $total_chars = 0;
    $trunc = false;
    $result = null;

    foreach ($words as $word) {
        if (strlen($result) + strlen($word) > $max_chars) {
            $trunc = true;
            break;
        }
        if ($result) {
            $result .= " $word";
        } else {
            $result = $word;
        }
    }

    if ($ellipsis && $trunc) {
        $result .= "...";
    }

    return $result;
}

// use this for user RAC and result credit
//
function format_credit($x) {
    return number_format($x, 2);
}

// use this when credit is likely to be large, e.g. team RAC
//
function format_credit_large($x) {
    return number_format($x, 0);
}

function friend_links($user) {
    if (is_banished($user)) {
        return "";
    }
    $x = "<table height=\"100\" width=\"150\" border=\"0\" cellpadding=\"4\"><tr><td class=\"friend\">";
    if ($user->has_profile) {
        $profile = BoincProfile::lookup_fields("has_picture", "userid=$user->id");
        if ($profile && $profile->has_picture) {
            $img_url = profile_thumb_url($user->id);
        } else {
            $img_url = URL_BASE."img/head_20.png";
        }
        $title = tra("View the profile of %1", $user->name);
        $alt = tra("Profile");
        $x .= ' <a href="'.URL_BASE.'view_profile.php?userid='.$user->id.'"><img title="'.$title.'" src="'.$img_url.'" alt="'.$alt.'"></a><br>';
    }
    $x .= " <a href=\"".URL_BASE."show_user.php?userid=".$user->id."\">".$user->name."</a>";
    if ($user->donated == 1) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK;
    }
    $x .= "</td></tr></table>\n";
    return $x;
}

function user_links($user) {
    BoincForumPrefs::lookup($user);
    if (is_banished($user)) {
        return "(banished: ID $user->id)";
    }
    $x = "";
    if ($user->has_profile) {
        $img_url = URL_BASE."img/head_20.png";
        $x .= ' <a href="'.URL_BASE.'view_profile.php?userid='.$user->id.'"><img title="View the profile of '.$user->name.'" src="'.$img_url.'" alt="Profile"></a>';
    }
    $x .= " <a href=\"".URL_BASE."show_user.php?userid=".$user->id."\">".$user->name."</a>";
    if ($user->donated == 1) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK;
    }
    if (function_exists("project_user_links")){
        $x .= project_user_links($user);
    }
    return $x;
}

function host_link($hostid) {
    if ($hostid) {
        return "<a href=\"show_host_detail.php?hostid=$hostid\">$hostid</a>";
    } else {
        return "---";
    }
}

function open_output_buffer() {
    ob_start();
    ob_implicit_flush(0);
}

function close_output_buffer($filename) {
    $fh = fopen($filename, "w");
    $page = ob_get_contents();
    ob_end_clean();
    fwrite($fh, $page);
    fclose($fh);
}

function html_info() {
    return "<br><a href=bbcode.php target=new><span class=smalltext>".tra("Use BBCode tags to format your text")."</span></a>\n";
}

// strip slashes if magic quotes in effect
function undo_magic_quotes($x) {
    if (get_magic_quotes_gpc()) {
        return stripslashes($x);
    }
    return $x;
}

// check for bogus GET args
//
function check_get_args($args) {
    foreach ($_GET as $key => $val) {
        if (!in_array($key, $args)) {
            Header("Location: extra_arg_$key.html");
            die;
        }
    }
}

function get_int($name, $optional=false) {
    $x=null;
    if (isset($_GET[$name])) $x = $_GET[$name];
    if (!is_numeric($x)) {
        if ($optional) {
            if ($x) {
                Header("Location: non_num_arg.html");
                die;
            }
            return null;
        } else {
            Header("Location: missing_arg_$name.html");
            die;
        }
    }
    return (int)$x;
}

function post_int($name, $optional=false) {
    $x = null;
    if (isset($_POST[$name])) $x = $_POST[$name];
    if (!is_numeric($x)) {
        if ($optional) {
            return null;
        } else {
            error_page("missing or bad parameter: $name; supplied: ".htmlspecialchars($x));
        }
    }
    return (int)$x;
}

function get_str($name, $optional=false) {
    if (isset($_GET[$name])) {
        $x = $_GET[$name];
    } else {
        if (!$optional) {
            error_page("missing or bad parameter: $name");
        }
        $x = null;
    }
    return undo_magic_quotes($x);
}

function post_str($name, $optional=false) {
    if (isset($_POST[$name])) {
        $x = $_POST[$name];
    } else {
        if (!$optional) {
            error_page("missing or bad parameter: $name");
        }
        $x = null;
    }
    return undo_magic_quotes($x);
}

function is_ascii($str) {
    // the mb_* functions are not included by default
    // return (mb_detect_encoding($passwd) -= 'ASCII');

    for ($i=0; $i<strlen($str); $i++) {
        $c = ord(substr($str, $i));
        if ($c < 32 || $c > 127) return false;
    }
    return true;
}

// This function replaces some often made mistakes while entering numbers
// and gives back an error if there are false characters
// It will also be checked if the value is within certain borders
// @param string &$value reference to the value that should be verified
// @param double $low the lowest number of value if verified
// @param double $high the highest number of value if verified
// @return bool true if $value is numeric and within the defined borders,
//   false if $value is not numeric, no changes were made in this case
//
function verify_numeric(&$value, $low, $high = false) {
    $number = trim($value);
    $number = str_replace('o', '0', $number);
    $number = str_replace('O', '0', $number);
    $number = str_replace('x', '', $number); //if someone enters '0x100'
    $number = str_replace(',', '.', $number); // replace the german decimal separator
    // if no value was entered and this is ok
    if ($number=='' && $low=='') return true;

    // the supplied value contains alphabetic characters
    if (!is_numeric($number)) return false;

    //if ($number < $low) $number = $low;
    if ($number < $low) return false;

    if ($high) {
        //if ($number > $high) $number = $high;
        if ($number > $high) return false;
    }
    $value = (double)$number;
    return true;
}

// Generate a "select" HTML element from an array of values
function select_from_array($name, $array, $selection) {
    $out = "<select name=\"$name\">";

    foreach ($array as $key => $value) {
        if ($value) {
            $out .= "<option ";
            if ($key == $selection) {
                $out .= "selected ";
            }
            $out .= "value=\"".$key."\">".$value."</option>";
        }
    }
    $out.= "</select>";
    return $out;
}

// Convert to entities, while preserving already-encoded entities.
// Do NOT use if $str contains valid HTML tags.
//
function boinc_htmlentities($str) {
    $str = html_entity_decode($str, ENT_COMPAT, "UTF-8");
    $str = htmlentities($str, ENT_COMPAT, "UTF-8");
    return $str;
}

function strip_bbcode($string){
    return preg_replace("/((\[.+\])+?)(.+?)((\[\/.+\])+?)/","",$string);
}

function current_url() {
    $url = "http";
    if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == "on") {
        $url .= "s";
    }
    $url .= "://";
    $url .= $_SERVER['SERVER_NAME'];
    $url .= ":".$_SERVER['SERVER_PORT'];
    if (isset($_SERVER['REQUEST_URI'])) {
        $url .= $_SERVER['REQUEST_URI'];
    } else {
        if ($_SERVER['QUERY_STRING']) {
            $url .= "?".$_SERVER['QUERY_STRING'];
        }
    }
    return $url;
}

// Show a single link formatted to look like a button.
// @param url The destination URL of the button
// @param text The text to display on the button
// @param desc The title of the destination - typically used as a popup
// @param class The optional CSS class of the button. Defaults to a standard button
//
function show_button($url, $text, $desc=null, $class="button") {
    if (!$desc) $desc = $text;
    echo "<a href=\"".$url."\" title=\"$desc\" class=\"".$class."\">".$text."</a>\n";
}
function button_text($url, $text, $desc=null, $class="button") {
    if (!$desc) $desc = $text;
    return "<a href=\"".$url."\" title=\"$desc\" class=\"".$class."\">".$text."</a>\n";
}

// When multiple buttons (or actions) are presented in a list you can
// use this convenience method to avoid having to wrap each button in <li></li> elements
// @param url The destination URL of the button
// @param text The text to display on the button
// @param desc The title of the destination - typically used as a popup
// @param class The optional CSS class of the button. Defaults to a standard button
//
function show_actionlist_button($url, $text, $desc, $class="button"){
    echo "<li>";
    echo show_button($url, $text, $desc, $class);
    echo "</li>";
}

function show_image($src, $title, $alt, $height=null) {
    $h = "";
    if ($height) {
        $h = "height=\"$height\"";
    }
    echo "<img border=\"0\" title=\"$title\" alt=\"$alt\" src=\"$src\" $h>";
}

function check_web_stopped() {
    global $generating_xml, $did_page_head;
    if (web_stopped()) {
        if ($generating_xml) {
            xml_error(-183);
        } else {
			if (!$did_page_head) {
				page_head(tra("Project down for maintenance"));
			}
            echo tra(
				"%1 is temporarily shut down for maintenance.  Please try again later.",
				PROJECT
			);
            page_tail();
            exit();
        }
    }
}

// Connects to database server and selects database as noted in config.xml
// If only read-only access is necessary,
// tries instead to connect to <replica_db_host> if tag exists.
// DEPRECATED - use boinc_db.inc
//
function db_init($try_replica=false) {
    check_web_stopped();
    $retval = db_init_aux($try_replica);
    if ($retval == 1) {
        echo tra("Unable to connect to database - please try again later");
        echo tra("Error:"), " ", mysql_errno(), mysql_error();
        exit();
    }
    if ($retval == 2) {
        echo tra("Unable to select database - please try again later");
        echo mysql_error();
        exit();
    }
    return 0;
}

// return a structure indicating whether project has non-deprecated apps
// for various resource types.
//
function get_app_types() {
    $t = null;
    $t->cpu = false;
    $t->cuda = false;
    $t->ati = false;
    $t->count = 0;
    $avs = BoincAppVersion::enum("deprecated=0");
    foreach ($avs as $av) {
        if (strstr($av->plan_class, "ati")) {
            $t->ati = true;
            $t->count++;
        } else if (strstr($av->plan_class, "cuda")) {
            $t->cuda = true;
            $t->count++;
        } else if (strstr($av->plan_class, "nvidia")) {
            $t->cuda = true;
            $t->count++;
        } else {
            $t->cpu = true;
            $t->count++;
        }
    }
    return $t;
}

// Functions to sanitize GET and POST args

// "next_url" arguments (must be local, not full URLs)
//
function sanitize_local_url($x) {
    $x = trim($x, "/");
    if (strstr($x, "/")) return "";
    if (strstr($x, "<")) return "";
    if (strstr($x, "\"")) return "";
    return $x;
}

// strip HTML tags
//
function sanitize_tags($x) {
    return strip_tags($x);
}

function sanitize_numeric($x) {
    if (is_numeric($x)) {
        return $x;
    } else {
        return "not numeric";
    }
}

function sanitize_email($x) {
    if (function_exists('filter_var')) {
        return filter_var($x, FILTER_SANITIZE_EMAIL);
    } else {
        return strip_tags($x);
    }
}

function flops_to_credit($f) {
    return $f*(200/86400e9);
}

function credit_to_gflop_hours($c) {
    return $c/(200/24);
}
function do_download($path,$name="") {
    if (strcmp($name,"") == 0) {
        $name=basename($path);
    }
    header('Content-Description: File Transfer');
    header('Content-Type: application/octet-stream');
    header('Content-Disposition: attachment; filename='.$name);
    header('Content-Transfer-Encoding: binary');
    header('Expires: 0');
    header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
    header('Pragma: public');
    header('Content-Length: ' . filesize($path));
    flush();
    readfile($path);
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
