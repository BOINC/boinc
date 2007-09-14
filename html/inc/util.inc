<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../project/project.inc");
require_once("../inc/countries.inc");
require_once("../inc/db.inc");
require_once("../inc/translation.inc");

$generating_xml = false;

// db_init
// Connects to database server and selects database as noted in config.xml
// (Or, in some cases, if only read-only access is necessary, tries instead
// to connect to replica database server noted in <replica_db_host> tag. If
// this tag doesn't exist, db_init will revert to connecting to <db_host>.)
//
function db_init($try_replica=false) {
    if (web_stopped()) {
        page_head("Not available");
        echo "This page requires database access.
            Our database server is temporarily shut down for maintenance.
            Please try again later.
        ";
        page_tail();
        exit();
    }
    $retval = db_init_aux($try_replica);
    if ($retval == 1) {
        echo "Unable to connect to database - please try again later\n";
        echo "Error: ", mysql_errno(), mysql_error();
        exit();
    }
    if ($retval == 2) {
        echo "Unable to select database - please try again later";
        echo mysql_error();
        exit();
    }
    return 0;
}

// Initializes the session and returns the authenticator
// for the session (if any)
//
function init_session() {
    $master_url = parse_config(get_config(), "<master_url>");
    $url = parse_url($master_url);
    $path = $url['path'];
    if (strlen($path)) {
        session_set_cookie_params(0, $path);
    }
    if (session_id() == "") session_start(); // first check if there is a session
// NOTE: in PHP 4.1+, s/key_exists/array_key_exists/
    if (array_key_exists('authenticator', $_SESSION)) {
        return $_SESSION["authenticator"];
    } else {
        return NULL;
    }
}

// if not logged in, put up login form and exit
//
function require_login($user) {
    if (!$user) {
        print_login_form();
        exit();
    }
}

function get_user_from_auth($auth) {
    if ($auth) return lookup_user_auth($auth);
    return NULL;
}

function get_user_from_id($id) {
    if ($id) return lookup_user_id($id);
    return NULL;
}

$g_logged_in_user = null;

function get_logged_in_user($must_be_logged_in=true) {
    global $g_logged_in_user;
    $authenticator = init_session();
    if (!$authenticator) {
        if (isset($_COOKIE['auth'])) $authenticator = $_COOKIE['auth'];
    }
    $authenticator = process_user_text($authenticator);
    $g_logged_in_user = get_user_from_auth($authenticator);
    if ($must_be_logged_in) {
        require_login($g_logged_in_user);
    }
    return $g_logged_in_user;
}

function show_login($user) {
    if ($user) {
        echo "Logged in as %s.\n", $user->name;
        echo "<br><a href=login_form.php>Log in as someone else.</a>\n";
    } else {
        echo "Not logged in";
    }
}

// Page_head() is overridable so that projects that want to integrate BOINC
// with an existing web framework can more easily do so.
// To take advantage of this simply define the function page_head()
// somewhere in the project include file.
//
if (!function_exists("page_head")){
function page_head($title, $java_onload=null, $title_plain=null, $prefix="") {
    $styleSheet = URL_BASE . "/".STYLESHEET;
    $rssname = PROJECT . " RSS 2.0";
    $rsslink = URL_BASE . "/rss_main.php";

    if (defined("CHARSET")) {
        header("Content-type: text/html; charset=".tr(CHARSET));
    }

//    echo '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">';

    if (!$title_plain) {
        echo "<html><head><title>".strip_tags($title)."</title>
            <link rel=\"stylesheet\" type=\"text/css\" href=\"$styleSheet\">
            <link rel=\"alternate\" type=\"application/rss+xml\" title=\"$rssname\" href=\"$rsslink\">
            </head>";
    } else {
        echo "<html><head><title>".strip_tags($title_plain)."</title>
        <link rel=\"stylesheet\" type=\"text/css\" href=\"$styleSheet\">
        <link rel=\"alternate\" type=\"application/rss+xml\" title=\"$rssname\" href=\"$rsslink\">
        </head>";
    }
    if ($java_onload){
        echo "<body bgcolor=ffffff onload=\"".$java_onload."\">"; } else { echo "<body bgcolor=ffffff>";
    }
    display_cvs_versions();
    project_banner($title, $prefix);
}
}

function page_tail_aux($show_return, $show_date, $prefix="") {
    project_footer($show_return, $show_date, $prefix);
    echo "</body>\n</html>";
}
function page_tail_main($show_date=false) {
    page_tail_aux(false, $show_date);
}

/**
 * See the comments for page_head()
 */
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
    echo "A database error occurred while handling your request.
        <br>Please try again later.
        <br>If the error persists, please submit a
        <a href=bug_report_form.php>problem report</a>.
    ";
    page_tail();
}

function error_page($msg) {
    global $generating_xml;
    if ($generating_xml) {
        xml_error(-1, $msg);
    }
    page_head("Unable to handle request");
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
        $datestring .= "$days days ";
    }
    if ($hours || strlen($datestring)) {
        $datestring .= "$hours hours ";
    }
    if ($minutes || strlen($datestring)) {
        $datestring .= "$minutes min ";
    }
    if ($seconds) {
        $datestring .= "$seconds sec";
    }

    return $datestring;
}


function date_str($x) {
    if ($x == 0) return "---";
    return gmdate('j M Y', $x);
}

function time_str($x) {
    if ($x == 0) return "---";
    return gmdate('j M Y G:i:s', $x) . " UTC";
}

function pretty_time_str($x) {
    return time_str($x);
}
function start_table($extra="width=100%") {
    echo "<table border=1 cellpadding=5 $extra>";
}

function start_table_noborder($width="100%") {
    echo "<table border=0 cellpadding=5 width=$width>";
}

function end_table() {
    echo "</table>\n";
}

/*
 * Table header row with unlimited number of columns
 */
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

/*
 * Table row with unlimited number of columns
 */
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
    echo "<tr><td class=$class colspan=$ncols>$x</td></tr>\n";
}

function row2($x, $y, $show_error=false) {
    if ($x=="") $x="<br>";
    if ($y=="") $y="<br>";
    if ($show_error) {
        $class1 = 'fieldname_error';
        $class2 = 'fieldvalue_error';
    } else {
        $class1 = 'fieldname';
        $class2 = 'fieldvalue';
    }
    echo "<tr><td width=40% class=$class1>$x</td><td class=$class2>$y</td></tr>\n";
}

function row2_init($x, $y) {
    echo "<tr><td class=fieldname width=40%>$x</td><td valign=top><b>$y\n";
}

function row2_plain($x, $y) {
    echo "<tr><td>$x</td><td>$y</td></tr>\n";
}

function row3($x, $y, $z) {
    echo "<tr><td width=30% valign=top align=right>$x</td><td>$y</td><td>$z</td></tr>\n";
}

function row4($xx, $xy, $yx, $yy) {
    echo "<tr><td width=25% valign=top>$xx</td><td width=25%>$xy</td>"
        . "<td width=25% >$yx</td><td width=%25>$yy</td></tr>
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
        echo "<th class=heading>$h</th>";
    }
    echo "</tr>\n";
}

function random_string() {
    return md5(uniqid(rand(), true));
}

function url_tokens($auth) {
    $now = time();
    $ttok = md5((string)$now.$auth);
    return "&tnow=$now&ttok=$ttok";
}

function form_tokens($auth) {
    $now = time();
    $ttok = md5((string)$now.$auth);
    return "<input type=hidden name=tnow value=$now>
        <input type=hidden name=ttok value=$ttok>
    ";
}

function valid_tokens($auth) {
    $tnow = $_GET['tnow'];
    $ttok = $_GET['ttok'];
    if (!$tnow) { $tnow = $_POST['tnow']; }
    if (!$ttok) { $ttok = $_POST['ttok']; }
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
        "Link has timed out.  Please click Back, refresh the page,
        and try again."
    );
}

function print_login_form_aux($next_url, $user) {
    echo "
        <form name=f method=post action=login_action.php>
        <input type=hidden name=next_url value='$next_url'>
    ";
    start_table();
    row1("Log in with email/password");
    row2("Email address:", '<input name="email_addr" size="40" tabindex="1">');
    row2("Password:<br><font size=-2><a href=edit_passwd_form.php>Forgot password?</a>",
        '<input type="password" name="passwd" size="40" tabindex="2">'
    );
    row2("", '<input type="submit" name="mode" value="Log in with email/password" tabindex="3">');
    row1("Log in with account key");
    row2("Your account key:
        <br><font size=-2>
        If you don't know your account key,
        <a href=get_passwd.php>click here</a>.
        </font>",
        "<input name=authenticator size=40>"
    );
    row2("", "<input type=submit name=mode value='Log in with account key'>");
    row1("Stay logged in");
    row2("Stay logged in on this computer",
        "<input type=checkbox name=send_cookie checked>"
    );
    if ($user) {
        row1("Log out");
        row2("You are logged in as $user->name",
            "<a href=logout.php?".url_tokens($user->authenticator).">Log out</a>"
        );
    }
    end_table();
    echo "
        </form>
        <script>
            document.f.email_addr.focus();
        </script>
    ";
}

function print_login_form() {
    page_head("Please log in");
    echo "
        This function requires that you log in.
        <p>
    ";
    $next_url = $_SERVER['REQUEST_URI'];
    print_login_form_aux($next_url, null);
    page_tail();
}

// Look for an element in a line of XML text
// If it's a single-tag element, and it's present, just return the tag
//
function parse_element($xml, $tag) {
    $element = null;
    $closetag = "</" . substr($tag,1);
    $x = strstr($xml, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, $closetag);
        if ($n) {
            $element = substr($y, 0, $n);
        }
    }
    return trim($element);
}

function parse_next_element($xml, $tag, &$cursor) {
    $element = null;
    $closetag = "</" . substr($tag,1);
    $pos = substr($xml,$cursor);
    $x = strstr($pos, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, $closetag);
        if ($n) {
            $element = substr($y, 0, $n);
        }
        $cursor = (strlen($xml) - strlen($x)) + strlen($tag) + strlen($closetag) + strlen($element);
    }
    return trim($element);
}

if (!function_exists("file_get_contents")) {
function file_get_contents($path) {
    $x = "";
    $f = fopen($path, "r");
    if ($f) {
        while (!feof($f)) $x .= fread($f, 4096);
        fclose($f);
    }
    return $x;
}
}

$g_config = null;
function get_config() {
    global $g_config;
    if ($g_config == null) {
       $g_config = file_get_contents("../../config.xml");
    }
    return $g_config;
}

// look for a particular element in the ../../config.xml file
//
function parse_config($config, $tag) {
    $element = parse_element($config, $tag);
    return $element;
}

// return true if XML contains either <tag/> or <tag>1</tag>
//
function parse_bool($xml, $tag) {
    $x = "<$tag/>";
    if (strstr($xml, $x)) return true;
    $x = "<$tag>";
    $y = (int)parse_element($xml, $x);
    if ($y != 0) return true;
    return false;
}

// Call this if for dynamic pages
//
function no_cache() {
    header ("Expires: Mon, 26 Jul 1997 05:00:00 UTC");    // Date in the past
    header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " UTC"); // always modified
    header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
    header ("Pragma: no-cache");                          // HTTP/1.0
}

// Generates a legal filename from a parameter string.

function get_legal_filename($name) {
    $name = ereg_replace(',', '', $name);
    return ereg_replace(' ', '_', $name);
}

// Returns a string containing as many words
// (being collections of characters separated by the character $delimiter)
// as possible such that the total string length is <= $chars characters long.
// If $ellipsis is true, then an ellipsis is added to any sentence which
// is cut short.

function sub_sentence($sentence, $delimiter, $max_chars, $ellipsis=false) {
    $words = explode($delimiter, $sentence);
    $total_chars = 0;
    $count = 0;
    $result = '';

    do {
        if ($count > 0) {
            $result = $result . ' ' . $words[$count];
        } else {
            $result = $result . $words[$count];
        }
        $total_chars += strlen($words[$count]) + 1;
        $count++;
    } while ($count < count($words) && ($total_chars + strlen($words[$count])) <= $max_chars);

    if ($ellipsis && ($count < count($words))) {
        $result = $result . '...';
    }

    return $result;
}

function format_credit($x) {
    return number_format($x, 2);
}

function web_stopped() {
    return file_exists("../../stop_web");
}

function sched_stopped() {
    return file_exists("../../stop_sched");
}

function user_links($user) {
    $x = "<a href=\"".URL_BASE."/show_user.php?userid=".$user->id."\">".$user->name."</a>";
    if ($user->has_profile) {
        $x .= ' <a href="'.URL_BASE.'/view_profile.php?userid='.$user->id.'"><img border="0" src="'.URL_BASE.'/img/head_20.png" alt="User profile image"></a>';
    }
    # Does this project accept donations?
    # If so, do you want to have a link next to user name as it appears on the web site?
    if ($user->donated == 1) { 
        require_once("../project/donations.inc");
        $x .= DONATION_LINK;
    }
    return $x;
}

function host_link($hostid) {
    if ($hostid) {
        return "<a href=show_host_detail.php?hostid=$hostid>$hostid</a>";
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
    return "<br><a href=\"bbcode.php\" target=\"_new\"><font size=-2>".tr(FORUM_HTML_INFO)."</font></a>\n";
}

function get_int($name, $optional=false) {
    $x=null;
    if (isset($_GET[$name])) $x = $_GET[$name];
    if (!is_numeric($x)) {
        if ($optional) {
            return null;
        } else {
            error_page("missing or bad parameter: $name; supplied: ".htmlspecialchars($x));
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
    $x = null;
    if (isset($_GET[$name])) $x = $_GET[$name];
    if (!$x && !$optional) {
        error_page("missing or bad parameter: $name");
    }
    return $x;
}

function get_venue($name) {
    $x = $_GET[$name];
    if ($x == "") return $x;
    if ($x == "home") return $x;
    if ($x == "work") return $x;
    if ($x == "school") return $x;
    error_page("no such venue: $x");
}

function post_str($name, $optional=false) {
    $x = null;
    if (isset($_POST[$name])) $x = $_POST[$name];
    if (!$x && !$optional) {
        error_page("missing or bad parameter: $name");
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

// This function takes as input a GET variable name X and value Y.
// It returns the corresponding text GET variable string, appended with
// any existing GET variable names and values, with &X=Y.
//
// This is useful for constructing urls for sorting a table by different
// columns.
//
function make_GET_list($variable_name, $variable_value) {
    $retval="";
    $sepchar='?';
    $modified=false;
    foreach ($_GET as $key => $value) {
        $retval .= "$sepchar"."$key=";
        $sepchar='&';
        if ($key==$variable_name) {
            $modified=true;
            if ($value!=$variable_value) {
                $retval .= "$variable_value";
            } else {
                $retval .= "$variable_value"."_reversed";
            }
        }
        else {
            $retval .= "$value";
        }
    }
    if (!$modified) $retval .= "$sepchar$variable_name=$variable_value";
    return $retval;
}

function link_with_GET_variables($text, $baseurl, $variable_name, $variable_value) {
        $list=make_GET_list($variable_name, $variable_value);
        return "<a href=\"$baseurl$list\">$text</a>";
}

// This function replaces some often made mistakes while entering numbers
// and gives back an error if there are false characters
// It will also be checked if the value is within certain borders
// @param string &$value reference to the value that should be verified
// @param double $low the lowest number of value if verified
// @param double $high the highest number of value if verified
// @return bool true if $value is numeric and within the defined borders, false if $value is not numeric, no changes were made in this case
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

    if ($number < $low) $number = $low;
    if ($high) {
        if ($number > $high) $number = $high;
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

?>
