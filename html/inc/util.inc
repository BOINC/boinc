<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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

// Utility functions for BOINC web pages
// Stuff that's not web-specific (e.g. that's used in non-web scripts)
// should go in util_basic.inc

require_once("../inc/util_basic.inc");
require_once("../project/project.inc");
require_once("../inc/countries.inc");
require_once("../inc/db.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/translation.inc");
require_once("../inc/profile.inc");
require_once("../inc/bootstrap.inc");

function master_url() {
    return parse_config(get_config() , "<master_url>");
}
function recaptcha_public_key() {
    return parse_config(get_config(), "<recaptcha_public_key>");
}
function recaptcha_private_key() {
    return parse_config(get_config(), "<recaptcha_private_key>");
}

// Set parameters to defaults if not defined in config.xml

$x = parse_config(get_config(), "<user_country>");
define('USER_COUNTRY', ($x===null)?1:(int)$x);

$x = parse_config(get_config(), "<user_url>");
define('USER_URL', ($x===null)?1:(int)$x);

// Set parameters to defaults if not defined in project.inc

if (defined('TIMEZONE')) {
    date_default_timezone_set(TIMEZONE);
} else {
    date_default_timezone_set('UTC');
}
if (!defined('DISABLE_PROFILES')) {
    define('DISABLE_PROFILES', false);
}
if (!defined('DISABLE_FORUMS')) {
    define('DISABLE_FORUMS', false);
}
if (!defined('DISABLE_TEAMS')) {
    define('DISABLE_TEAMS', false);
}
if (!defined('DISABLE_BADGES')) {
    define('DISABLE_BADGES', false);
}
if (!defined('BADGE_HEIGHT_SMALL')) {
    define('BADGE_HEIGHT_SMALL', 20);
}
if (!defined('BADGE_HEIGHT_MEDIUM')) {
    define('BADGE_HEIGHT_MEDIUM', 24);
}
if (!defined('BADGE_HEIGHT_LARGE')) {
    define('BADGE_HEIGHT_LARGE', 56);
}
if (!defined('LDAP_HOST')) {
    define('LDAP_HOST', null);
}
if (!defined('POSTAL_CODE')) {
    define('POSTAL_CODE', false);
}
if (!defined('NO_COMPUTING')) {
    define('NO_COMPUTING', false);
}
if (!defined('NO_HOSTS')) {
    define('NO_HOSTS', false);
}
if (!defined('NO_STATS')) {
    define('NO_STATS', false);
}
if (!defined('NO_GLOBAL_PREFS')) {
    define('NO_GLOBAL_PREFS', false);
}

// the 'home page' of the logged-in user.
// go here after login, account creation, team operations, etc.
//
if (!defined('HOME_PAGE')) {
    define('HOME_PAGE', 'home.php');
}

// the page showing another user ('userid' arg).
// Link to here wherever we show a user name.
//
if (!defined('SHOW_USER_PAGE')) {
    define('SHOW_USER_PAGE', 'show_user.php');
}

if (!defined('POST_MAX_LINKS')) {
    define('POST_MAX_LINKS', 0);
}

// support dark mode if user has selected it
if (!defined('DARK_MODE')) {
    define('DARK_MODE', true);
}
if (!defined('VALIDATE_EMAIL_TO_POST')) {
    define('VALIDATE_EMAIL_TO_POST', false);
}
if (!defined('UNIQUE_USER_NAME')) {
    define('UNIQUE_USER_NAME', false);
}

// don't allow anything between .php and ? in URL
//
if (array_key_exists("PATH_INFO", $_SERVER)) {
    die("bad URL");
}

// sleep this long on any login failure
// (slow the rate of hacker attacks)
//
define('LOGIN_FAIL_SLEEP_SEC', 5);

$caching = false;
    // if set, we're writing to a file rather than to client
$did_page_head = false;

define('KILO', 1024);
define('MEGA', 1024*KILO);
define('GIGA', 1024*MEGA);
define('TERA', 1024*GIGA);

// return true if this page is HTTPS
//
function is_https() {
    return isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'];
}

function secure_url_base() {
    if (defined('SECURE_URL_BASE')) return SECURE_URL_BASE;
    return URL_BASE;
}

function url_base() {
    return is_https()?secure_url_base():URL_BASE;
}

function send_cookie($name, $value, $permanent, $ops=false) {
    // the following allows independent login for projects on the same server
    //
    $url = parse_url(master_url());
    $path = $url['path'];
    if ($ops) {
        $path = substr($path, 0, -1);
        $path .= "_ops/";
    }
    $expire = $permanent?time()+3600*24*365:0;
    setcookie($name, $value, $expire, $path,
        '',
        is_https(),     // if this page is secure, make cookie secure
        true            // httponly; no JS access
    );
}

function clear_cookie($name, $ops=false) {
    $url = parse_url(master_url());
    $path = $url['path'];
    if ($ops) {
        $path = substr($path, 0, -1);
        $path .= "_ops/";
    }
    setcookie($name, '', time()-3600, $path);
}

$g_logged_in_user = null;
$got_logged_in_user = false;

function get_logged_in_user($must_be_logged_in=true) {
    global $g_logged_in_user, $got_logged_in_user;
    if ($got_logged_in_user) {
        // this could have been called earlier with $must_be_logged_in false
        if ($must_be_logged_in) {
            if ($g_logged_in_user) return $g_logged_in_user;
        } else {
            return $g_logged_in_user;
        }
    }

    if (web_stopped()) return null;

    $authenticator = null;
    if (isset($_COOKIE['auth'])) $authenticator = $_COOKIE['auth'];

    if ($authenticator) {
        $authenticator = BoincDb::escape_string($authenticator);
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
        Header("Location: ".url_base()."login_form.php?next_url=$next_url");
        exit;
    }
    $got_logged_in_user = true;
    return $g_logged_in_user;
}

function show_login_info($prefix="") {
    $user = get_logged_in_user(false);
    if ($user) {
        $url_tokens = url_tokens($user->authenticator);
        echo "<nobr>$user->name &middot; <a href=".$prefix."logout.php?$url_tokens>".tra("log out")."</a></nobr>";
    } else {
        echo "<a href=".$prefix."login_form.php>".tra("log in")."</a>";
    }
}

$cache_control_extra="";
$is_login_page = false;

// Call this to start pages.
// Outputs some HTML boilerplate,
// then calls project_banner() (in html/project/project.inc)
// to output whatever you want at the top of your web pages.
//
// Page_head() is overridable so that projects that want to integrate BOINC
// with an existing web framework can more easily do so.
// To do so, define page_head() in html/project/project.inc
//
if (!function_exists("page_head")){
function page_head(
    $title,
        // page title. Put in <title>, used as title for browser tab.
    $body_attrs=null,
        // <body XXXX>
        // e.g. Javascript to put focus in an input field
        // (onload="document.form.foo.focus()")
        // or to jump to a particular post (onload="jumpToUnread();")
    $is_main = false,
        // if set, include schedulers.txt.
        // also pass to project_banner() in case you want a different
        // header for your main page.
    $url_prefix="",
        // prepend this to links.
        // Use for web pages not in the top directory
    $head_extra=null
        // extra stuff to put in <head>. E.g.:
        // reCAPTCHA code (create_profile.php)
        // bbcode javascript (forums)
) {
    global $caching, $cache_control_extra, $did_page_head;
    global $is_login_page, $fixed_navbar;

    if ($did_page_head) {
        return;
    }

    $did_page_head = true;
    $url_base = url_base();

    $rssname = "RSS 2.0";
    $rsslink = $url_base."rss_main.php";

    if (!$caching) {
        header("Content-type: text/html; charset=utf-8");
        header("Expires: Mon, 26 Jul 1997 05:00:00 UTC");
            // Date in the past
        header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " UTC");
            // always modified
        header("Cache-Control: $cache_control_extra no-cache, must-revalidate, post-check=0, pre-check=0");
            // for HTTP/1.1
        header("Pragma: no-cache");
            // for HTTP/1.0
    }

    echo '<!DOCTYPE html>
        <html lang="en">
        <head>
    ';
    echo '<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>';

    if (defined('GLOBAL_HEAD_EXTRA')) {
        echo GLOBAL_HEAD_EXTRA;
    }
    echo '
        <meta name="viewport" content="width=device-width, initial-scale=1">
    ';
    if ($head_extra) {
        echo "\n$head_extra\n";
    }
    if ($is_main && (!defined('NO_COMPUTING')||!NO_COMPUTING)) {
        readfile("schedulers.txt");
    }

    $t = $title?$title:PROJECT;
    echo "<title>$t</title>\n";
    echo '
        <meta charset="utf-8">
    ';
    if (DARK_MODE) {
        echo <<<EOT
<script>
  if (window.matchMedia('(prefers-color-scheme: dark)').media === 'not all') {
      console.log("foobar");
    document.documentElement.style.display = 'none';
    document.head.insertAdjacentHTML(
        'beforeend',
        '<link rel="stylesheet" href="sample_bootstrap.min.css" onload="document.documentElement.style.display = \'\'"><link rel="stylesheet" href="custom.css">'
    );
  }
</script>
<link rel="stylesheet" href="bootstrap_darkly.min.css" media="(prefers-color-scheme: dark)">
<link rel="stylesheet" href="custom_dark.css" media="(prefers-color-scheme: dark)">
<link rel="stylesheet" href="sample_bootstrap.min.css" media="(prefers-color-scheme: no-preference), (prefers-color-scheme: light)">
<link rel="stylesheet" href="custom.css" media="(prefers-color-scheme: no-preference), (prefers-color-scheme: light)">
EOT;
    } else {
        echo '
            <link type="text/css" rel="stylesheet" href="'.secure_url_base().'/bootstrap.min.css" media="all">
        ';
    }
    if (defined('STYLESHEET')) {
        $stylesheet = $url_base.STYLESHEET;
        echo "
            <link rel=stylesheet type=\"text/css\" href=\"$stylesheet\">
        ";
    }
    if (defined('STYLESHEET2')) {
        $stylesheet2 = $url_base.STYLESHEET2;
        echo "
            <link rel=stylesheet type=\"text/css\" href=\"$stylesheet2\">
        ";
    }

    if (defined("SHORTCUT_ICON")) {
        echo '<link rel="icon" type="image/x-icon" href="'.SHORTCUT_ICON.'"/>
';
    }

    echo "
        <link rel=alternate type=\"application/rss+xml\" title=\"$rssname\" href=\"$rsslink\">
        </head>
    ";
    if ($fixed_navbar) {
        $body_attrs .= ' style="padding-top:70px"';
    }
    echo "<body $body_attrs>";
    echo '<div class="container-fluid">
    ';

    switch($title) {    //kludge
    case tra("Log in"):
    case tra("Create an account"):
    case tra("Server status page"):
        $is_login_page = true;
        break;
    default:
        break;
    }
    project_banner($title, $url_prefix, $is_main);
}
}

// See the comments for page_head()
//
if (!function_exists("page_tail")){
function page_tail(
    $show_date=false,
        // true for pages that are generated periodically rather than on the fly
    $url_prefix="",
        // use for pages not at top level
    $is_main=false
        // passed to project_footer;
) {
    echo "<br>\n";
    project_footer($is_main, $show_date, $url_prefix);
    echo '
        <script src="'.secure_url_base().'/bootstrap.min.js"></script>
        </div>
        </body>
        </html>
    ';
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

// convert time interval in seconds to a string of the form
// 'D days h hours m min s sec'.

function time_diff($x, $res=3) {
    $x = (int)$x;
    $days    = (int)($x/86400);
    $hours   = (int)(($x-$days*86400)/3600);
    $minutes = (int)(($x-$days*86400-$hours*3600)/60);
    $seconds = $x % 60;

    $s = "";
    if ($days) {
        $s .= "$days ".tra("days")." ";
    }
    if ($res>0 && ($hours || strlen($s))) {
        $s .= "$hours ".tra("hours")." ";
    }
    if ($res>1 && ($minutes || strlen($s))) {
        $s .= "$minutes ".tra("min")." ";
    }
    if ($res>2) {
        $s .= "$seconds ".tra("sec")." ";
    }
    return $s;
}

function start_table_str($class="", $style="") {
    $s = $style?'style="'.$style.'"':'';
    return '<div class="table">
      <table '.$s.' width="100%" class="table table-condensed '.$class.'" >
    ';
}

function start_table($class="", $style="") {
    echo start_table_str($class, $style);
}

function end_table_str() {
    return '</table>
        </div>
    ';
}

function end_table() {
    echo end_table_str();
}

// Table header row with unlimited number of columns

function table_header() {
    echo "<tr>\n";
    $c = 'class="bg-primary"';
    for ($i = 0; $i < func_num_args(); $i++) {
        if (is_array(func_get_arg($i))) {
            $col = func_get_arg($i);
            echo "<th $c ".$col[1].">".$col[0]."</th>\n";
        } else {
            echo "<th $c>".func_get_arg($i)."</th>\n";
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
    if ($class == "heading") {
        echo "<tr><th class=\"bg-primary\" colspan=\"$ncols\">$x</th></tr>\n";
    } else {
        echo "<tr><td class=\"$class\" colspan=\"$ncols\">$x</td></tr>\n";
    }
}

define('NAME_ATTRS', 'class="text-right " style="padding-right:12px"');
define('VALUE_ATTRS', 'style="padding-left:12px"');
define('VALUE_ATTRS_ERR', 'class="danger" style="padding-left:12px"');

// a table row with 2 columns, with the left on right-aligned

function row2($x, $y, $show_error=false, $lwidth='40%') {
    if ($x==="") $x="<br>";
    if ($y==="") $y="<br>";
    $attrs = $show_error?VALUE_ATTRS_ERR:VALUE_ATTRS;
    echo "<tr>
        <td width=\"$lwidth\" ".NAME_ATTRS.">$x</td>
        <td $attrs >$y</td>
        </tr>
    ";
}

// output the first part of row2();
// then write the content, followed by </td></tr>

function row2_init($x, $lwidth='40%') {
    echo sprintf('<tr>
        <td width="%s" %s>%s</td>
        <td %s>',
        $lwidth, NAME_ATTRS, $x, VALUE_ATTRS
    );
}

function row2_plain($x, $y) {
    echo "<tr><td>$x</td><td>$y</td></tr>\n";
}

function rowify($string) {
    echo "<tr><td>$string</td></tr>";
}

function row_array($x, $attrs=null) {
    echo "<tr>\n";
    $i = 0;
    foreach ($x as $h) {
        $a = $attrs?$attrs[$i]:"";
        echo "<td $a>$h</td>\n";
        $i++;
    }
    echo "</tr>\n";
}

define ('ALIGN_RIGHT', 'style="text-align:right;"');

function row_heading_array($x, $attrs=null, $class='bg-primary') {
    echo "<tr>";
    $i = 0;
    foreach ($x as $h) {
        $a = $attrs?$attrs[$i]:"";
        echo "<th $a class=\"$class\">$h</th>";
        $i++;
    }
    echo "</tr>\n";
}

function row_heading($x, $class='bg-primary') {
    echo sprintf('<tr><th class="%s" colspan=99>%s</th></tr>
        ', $class, $x
    );
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
      if (isset($_POST['tnow'])) {
          $tnow = $_POST['tnow'];
      }
    }
    if (!$ttok) {
      if (isset($_POST['ttok'])) {
          $ttok = $_POST['ttok'];
      }
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
    $result = "";

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

function bbcode_info() {
    return "<br><a href=bbcode.php target=new><small>".tra("Use BBCode tags to format your text")."</small></a>\n";
}

// strip slashes if magic quotes in effect
function undo_magic_quotes($x) {
    if (version_compare(PHP_VERSION, '5.4.0') < 0 && get_magic_quotes_gpc()) {
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

// returns null if the arg is optional and missing
//
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

// returns null if the arg is optional and missing
//
function post_num($name, $optional=false) {
    $x = null;
    if (isset($_POST[$name])) $x = $_POST[$name];
    if (!is_numeric($x)) {
        if ($optional) {
            return null;
        } else {
            error_page("missing or bad parameter: $name; supplied: ".htmlspecialchars($x));
        }
    }
    return (double)$x;
}

// returns null if the arg is optional and missing
//
function post_int($name, $optional=false) {
    $x = post_num($name, $optional);
    if (is_null($x)) return null;
    $y = (int)$x;
    if ($x != $y) {
        error_page("param $name must be an integer");
    }
    return $y;
}

function get_array($name) {
    if (isset($_GET[$name])) {
        return $_GET[$name];
    } else {
        return array();
    }
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

function post_arr($name, $optional=false) {
    if (isset($_POST[$name]) && is_array($_POST[$name])) {
        $x = $_POST[$name];
    } else {
        if (!$optional) {
            error_page("missing or bad parameter: $name");
        }
        $x = null;
    }
    return $x;
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
    //
    if ($number=='' && !$low) return true;

    // the supplied value contains alphabetic characters
    //
    if (!is_numeric($number)) return false;

    if ($number < $low) return false;

    if ($high) {
        if ($number > $high) return false;
    }
    $value = (double)$number;
    return true;
}

// Generate a "select" element from an array of values
//
function select_from_array($name, $array, $selection=null, $width=240) {
    $out = '<select style="color:#000;"class="form-control input-sm" style="width:'.$width.'px" name="'.$name.'">"';

    foreach ($array as $key => $value) {
        if ($value) {
            $out .= "<option ";
            if ($key == $selection) {
                $out .= "selected ";
            }
            $out .= "value=\"".$key."\">".$value."</option>\n";
        }
    }
    $out .= "</select>\n";
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
    $url = is_https()?'https':'http';
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

// return style for a button
// the colors for bootstrap's btn-success are almost illegible;
// the green is too light.  Use a darker green.
//
function button_style($color='green', $font_size=null) {
    $fs = '';
    if ($font_size) {
        $fs = sprintf('; font-size:%dpx', $font_size);
    }
    if ($color == 'green') $color = 'seagreen';
    if ($color == 'blue') $color = 'steelblue';
    return sprintf(
        'style="background-color:%s; color:white; text-decoration:none%s"',
        $color, $fs
    );
}
// Show a link formatted to look like a button.
// url: the destination of the lin
// text: The text to display on the button
// desc: tooltip text (show on hover)
// class: class of the button, e.g. btn
// extra: Additional text in href tag
//
function button_text($url, $text, $desc=null, $class=null, $extra='') {
    if (!$desc) {
        $desc = $text;
    }
    if (!$extra && !$class) {
        $extra = button_style();
    }
    if (!$class) {
        $class = "btn btn-sm";
    }
    return sprintf(' <a href="%s" title="%s" class="btn %s" %s>%s</a>',
        $url, $desc, $class, $extra, $text
    );
}

function button_text_small($url, $text, $desc=null) {
    return button_text($url, $text, $desc, "btn btn-xs", button_style());
}

function show_button($url, $text, $desc=null, $class=null, $extra=null) {
    echo button_text($url, $text, $desc, $class, $extra);
}

// for places with a bunch of buttons, like forum posts
//
function show_button_small($url, $text, $desc=null) {
    echo button_text_small($url, $text, $desc);
}

// used for showing icons
//
function show_image($src, $title, $alt, $height=null) {
    $h = "";
    if ($height) {
        $h = "height=\"$height\"";
    }
    echo "<img class=\"icon\" border=\"0\" title=\"$title\" alt=\"$alt\" src=\"$src\" $h>";
}

function show_project_down() {
    global $did_page_head;
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

function check_web_stopped() {
    global $generating_xml;
    if (web_stopped()) {
        if ($generating_xml) {
            xml_error(-183);
        } else {
            show_project_down();
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
        exit();
    }
    if ($retval == 2) {
        echo tra("Unable to select database - please try again later");
        exit();
    }
    return 0;
}

// Functions to sanitize GET and POST args

// "next_url" arguments (must be local, not full URLs)
//
function sanitize_local_url($x) {
    if (!$x) return $x;
    $x = trim($x, "/");
    if (strstr($x, "/")) return "";
    if (strstr($x, "<")) return "";
    if (strstr($x, "\"")) return "";
    return $x;
}

function sanitize_user_url($x) {
    // if protocol is given, must be http: or https:
    $y = explode('//', $x);
    switch (count($y)) {
    case 1:
        $z = $x;
        break;
    case 2:
        $z = $y[1];
        if (strtolower($y[0]) == 'http:') break;
        if (strtolower($y[0]) == 'https:') break;
        return '';
    default: return '';
    }
    // everything else must be alphanumeric, or -_/.#?&=
    if (preg_match('/^[-_.\/#?&=a-zA-Z0-9]*$/', $z)) {
        return $x;
    }
    return '';
}

// strip HTML tags
//
function sanitize_tags($x) {
    if (!$x) return $x;
    return strip_tags($x);
}

function sanitize_numeric($x) {
    if (!$x || !is_numeric($x)) {
        return '';
    }
    return $x;
}

function sanitize_email($x) {
    if (!$x) return $x;
    if (function_exists('filter_var')) {
        return filter_var($x, FILTER_SANITIZE_EMAIL);
    } else {
        return strip_tags($x);
    }
}

// pages like top_hosts.php, team_members.php etc. have a textual
// "sort_by" argument.
// Check this to avoid XSS vulnerability
//
function sanitize_sort_by($x) {
    switch($x) {
    case 'expavg_credit':
    case 'total_credit':
        return;
    default:
        error_page('bad sort_by');
    }
}

function flops_to_credit($f) {
    return $f*(200/86400e9);
}

function credit_to_gflop_hours($c) {
    return $c/(200/24);
}

function do_download($path) {
    $name=basename($path);
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

// if have SSL but not using it, redirect
//
function redirect_to_secure_url() {
    if (defined('SECURE_URL_BASE')
        && strstr(SECURE_URL_BASE, "https://")
        && !is_https()
    ) {
        Header("Location: https://".$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI']);
        exit;
    }
}

if (php_sapi_name() != "cli") {
    redirect_to_secure_url();
}

function badges_string($is_user, $item, $height) {
    if (DISABLE_BADGES) return null;
    if ($is_user) {
        $bus = BoincBadgeUser::enum("user_id=$item->id");
    } else {
        $bus = BoincBadgeTeam::enum("team_id=$item->id");
    }
    if (!$bus) return null;
    $x = "";
    foreach ($bus as $bu) {
        $badge = BoincBadge::lookup_id($bu->badge_id);
        $x .= "<img title=\"$badge->title\" valign=top height=$height src=$badge->image_url> ";
    }
    return $x;
}

function show_badges_row($is_user, $item) {
    if (BADGE_HEIGHT_LARGE == 0) return;
    $x = badges_string($is_user, $item, BADGE_HEIGHT_LARGE);
    if ($x) {
        row2("Badges", $x);
    }
}

// If this request is from a BOINC client, return its version as MMmmRR.
// Otherwise return 0.
// Format of user agent string is "BOINC client (windows_x86_64 7.3.17)"
//
function boinc_client_version(){
    if (!array_key_exists('HTTP_USER_AGENT', $_SERVER)) return 0;
    $x = $_SERVER['HTTP_USER_AGENT'];
    $e =  "/BOINC client [^ ]* (\d+).(\d+).(\d+)\)/";
    if (preg_match($e, $x, $matches)) {
        return $matches[1]*10000 + $matches[2]*100 + $matches[3];
    }
    return 0;
}

// output a script for counting chars left in text field
//
function text_counter_script() {
    echo "<script type=\"text/javascript\">
        function text_counter(field, countfield, maxlimit) {
            if (field.value.length > maxlimit) {
                field.value =field.value.substring(0, maxlimit);
            } else {
                countfield.value = maxlimit - field.value.length
            }
        }
        </script>
    ";
}

// return HTML for a textarea with chars-remaining counter.
// Call text_counter_script() before using this.
//
function textarea_with_counter($name, $maxlen, $text) {
    $rem_name = $name."_remaining";
    return "<textarea name=\"$name\" class=\"form-control\" rows=3 id=\"$name\" onkeydown=\"text_counter(this.form.$name, this.form.$rem_name, $maxlen);\"
        onkeyup=\"text_counter(this.form.$name, this.form.$rem_name, $maxlen);\">".$text."</textarea>
        <br><input name=\"$rem_name\" type=\"text\" id=\"$rem_name\" value=\"".($maxlen-strlen($text))."\" size=\"3\" maxlength=\"3\" readonly> ".tra("characters remaining")
    ;
}

// convert number MMmmrr to string MM.mm.rr
//
function version_string_maj_min_rel($v) {
    $maj = (int)($v/10000);
    $v -= $maj*10000;
    $min = (int)($v/100);
    $v -= $min*100;
    return sprintf("%d.%d.%d", $maj, $min, $v);
}

function google_search_form($url) {
    echo "
        <nobr>
        <form method=get action=\"https://google.com/search\">
        <input type=hidden name=domains value=\"$url\">
        <input type=hidden name=sitesearch value=\"$url\">
        <input name=q size=20>
        <input type=submit value=".tra("Search").">
        </form>
        </nobr>
    ";
}

// use the following around text with long lines,
// to limit the width and make it more readable.
//
function text_start($width=640) {
    echo sprintf("<div style=\"max-width: %dpx;\">\n", $width);
}
function text_end() {
    echo "</div>\n";
}

// express a size in terms of TB, GB etc.
//
function size_string($x) {
    if ($x > TERA) {
        return number_format($x/TERA, 2)." TB";
    }
    if ($x > GIGA) {
        return number_format($x/GIGA, 2)." GB";
    }
    if ($x > MEGA) {
        return number_format($x/MEGA, 2)." MB";
    }
    if ($x > KILO) {
        return number_format($x/KILO, 2)." KB";
    }
    return "$x bytes";
}

function cert_filename() {
    return defined("CERT_FILENAME")?CERT_FILENAME:"cert1.php";
}

// if user hasn't validated their email addr, tell them to
//
function check_validated_email($user) {
    if (!$user->email_validated) {
        page_head("Please validate your email address");
        echo "
            To post here, please
            <a href=validate_email_addr.php>validate your email address<a>.
        ";
        page_tail();
        exit;
    }
}

?>
