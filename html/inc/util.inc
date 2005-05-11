<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../project/project.inc");
require_once("../inc/countries.inc");
require_once("../inc/db.inc");

function db_init() {
    if (project_is_stopped()) {
        page_head("Not available");
        echo "This page requires database access.
            Our database server is temporarily shut down for maintenance.
            Please try again later.
        ";
        page_tail();
        exit();
    }
    $retval = db_init_aux();
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
}

// Initializes the session and returns the authenticator
// for the session (if any)
//
function init_session() {
    $url = parse_url(MASTER_URL);
    $path = $url['path'];
    if (strlen($path)) {
        session_set_cookie_params(0, $path);
    }
    session_start();
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

function get_logged_in_user($must_be_logged_in=true) {
    $authenticator = init_session();
    if (!$authenticator) {
        if (isset($_COOKIE['auth'])) $authenticator = $_COOKIE['auth'];
    }
    $authenticator = process_user_text($authenticator);
    $user = get_user_from_auth($authenticator);
    if ($must_be_logged_in) {
        require_login($user);
    }
    return $user;
}

function show_login($user) {
    if ($user) {
        echo "Logged in as %s.\n", $user->name;
        echo "<br><a href=login_form.php>Log in as someone else.</a>\n";
    } else {
        echo "Not logged in";
    }
}

function page_head($title, $java_onload="") {
    $styleSheet = URL_BASE . STYLESHEET;
    $rssname = PROJECT . " RSS 2.0";
    $rsslink = URL_BASE . "rss_main.php";

    if (defined("CHARSET")) {
        header("Content-type: text/html; charset=".tr(CHARSET));
    }

    echo "<html><head><title>".strip_tags($title)."</title>
        <link rel=stylesheet type=text/css href=\"$styleSheet\">
        <link rel='alternate' type='text/xml' title=\"$rssname\" href=\"$rsslink\" />
        </head>";
    if ($java_onload!=""){echo "<body bgcolor=ffffff onload=\"".$java_onload."\">"; } else { echo "<body bgcolor=ffffff>";}
    display_cvs_versions();
    project_banner($title);
}

function page_tail_aux($show_return, $show_date) {
    project_footer($show_return, $show_date);
    echo "</body>\n</html>";
}
function page_tail_main($show_date=false) {
    page_tail_aux(false, $show_date);
}

function page_tail($show_date=false) {
    page_tail_aux(true, $show_date);
}

function display_cvs_versions(){
    global $cvs_version_tracker;
    echo "\n<!-- CVS VERSIONS -->\n";
    for ($i=0;$i<sizeof($cvs_version_tracker);$i++){
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

function profile_error_page($str) {
    page_head("Profile error");
    echo "$str<br>\n";
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
}

function error_page($msg) {
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
	if ($days)
		$datestring .= "$days days ";
	if ($hours || strlen($datestring))
		$datestring .= "$hours hours ";
	if ($min || strlen($datestring))
		$datestring .= "$minutes min ";
	if ($seconds)
		$datestring .= "$seconds sec";

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

function row1($x, $ncols=2, $class="heading") {
    echo "<tr><td class=$class colspan=$ncols>$x</td></tr>\n";
}

function row2($x, $y) {
    if ($x=="") $x="<br>";
    if ($y=="") $y="<br>";
    echo "<tr><td width=40% class=fieldname>$x</td><td class=fieldvalue>$y</td></tr>\n";
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
        echo "<td>$h</th>";
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

function print_login_form_aux($next_url, $user) {
    echo "
        <form name=f method=post action=login_action.php>
        <input type=hidden name=next_url value='$next_url'>
    ";
    start_table();
    row1("Log in");
    row2("Your account key:
        <br><font size=-2>
        If you don't know your account key,
        <a href=get_passwd.php>click here</a>.
        </font>",
        "<input name=authenticator size=40>"
    );
    row2("Remember account key on this computer",
        "<input type=checkbox name=send_cookie>"
    );
    row2("",
        "<input type=submit value='Log in'>"
    );
    if ($user) {
        row1("Log out");
        row2("You are logged in as $user->name",
            "<a href=logout.php>Log out</a>"
        );
    }
    end_table();
    echo "
        </form>
        <script>
            document.f.authenticator.focus();
        </script>
    ";
}

function print_login_form() {
    page_head("Please log in");
    echo "
        This function requires that you log in.
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
    $x = strstr($xml, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, "<");
        if ($n) {
            $element = substr($y, 0, $n);
        }
    }
    return $element;
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
    return trim($element);
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

function project_is_stopped() {
    if (file_exists("../../stop_web")) return true;
    return false;
}

function user_links($user) {
    $x = "<a href=show_user.php?userid=$user->id>$user->name</a>";
    if ($user->has_profile) {
        $x .= " <a href=view_profile.php?userid=$user->id><img border=0 src=head_20.png></a>";
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
    return "<br><a href=html.php><font size=-2>You may use HTML tags</font></a>\n";
}

function get_int($name, $optional=false) {
    $x = $_GET[$name];
    if (!is_numeric($x)) {
        if ($optional) {
            return null;
        } else {
            error_page("missing or bad parameter: $name $x");
        }
    }
    return (int)$x;
}

function post_int($name, $optional=false) {
    $x = $_POST[$name];
    if (!is_numeric($x)) {
        if ($optional) {
            return null;
        } else {
            error_page("missing or bad parameter: $name $x");
        }
    }
    return (int)$x;
}

function get_str($name, $optional=false) {
    $x = $_GET[$name];
    if (!$x && !optional) {
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
    $x = $_POST[$name];
    if (!$x && !optional) {
        error_page("missing or bad parameter: $name");
    }
    return $x;
}

?>
