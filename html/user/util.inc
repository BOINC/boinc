<?php

require_once("db.inc");
require_once("project.inc");

define("EMAIL_EXISTS", -1);
define("EMAIL_UPDATED", 1);
define("EMAIL_FAIL", -2);

define("LG_FONT", "<font size=+1>");
define("SM_FONT", "<font size=-1>");

define("TD", "<td bgcolor=cccccc>");
define("TD2", "<td colspan=2 bgcolor=cccccc>");
define("TD3", "<td colspan=3 bgcolor=cccccc>");

define("TABLE", "<table cellpadding=10 cellspacing=4 border=0 width=100%>");
define("VISTABLE", "<table cellpadding=2 cellspacing=0 border=1 width=100%>");
define("TABLE2", "<table width=580 cellpadding=3>");

define("BG_COLOR", " bgcolor=ffffff ");
define("TITLE_COLOR", " bgcolor=000000 ");
define("TITLE_FONT", " <font color=ffffff> ");
define("BODY_COLOR", " bgcolor=ffffff ");

// Sends the authenticator to the given email address
//
function send_auth_email($email_addr, $auth) {
    mail($email_addr, PROJECT." new account confirmation",
"This email confirms the creation of your ".PROJECT." account.
".PROJECT." URL:          ".MASTER_URL."
Your account key:            $auth\n
Please save this email, as you will need the account key
to access your account on the ".PROJECT." web site."
    );
}

// Initializes the session and returns the authenticator
// for the session (if any)
//
function init_session() {
    session_start();
    if (!isset($_SESSION["authenticator"])) {
	$_SESSION["authenticator"] = $authenticator;
    }
    return $_SESSION["authenticator"];
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

function show_login($user) {
    if ($user) {
        echo "Logged in as %s.\n", $user->name;
        echo "<br><a href=login_form.php>Log in as someone else.</a>\n";
    } else {
	echo "Not logged in";
    }
}

function page_head($title, $user=null) {
    echo "<head><title>$title</title><body ".BG_COLOR.">\n";

    project_banner($user);

}

function page_tail() {

    echo "<br><hr><center><a href=index.php>Return to ".PROJECT." main page</a><br>\n";

    // put your copyright notice etc. here

    echo "<br><br>Copyright (c) 2003 ".PROJECT."</center>\n";
}

function date_str($when) {
    return date("g:i A, l M j", $when);
}

function time_str($x) {
    if ($x == 0) return "---";
    return strftime("%T %b %e, %Y", $x);
}

function start_table() {
    echo "<table width=600 border=2 cellpadding=4>";
}

function row($x, $y) {
    echo "<tr><td width=30% valign=top align=right>$x</td><td>$y</td></tr>\n";
}

function row2($x, $y) {
    echo "<tr><td>$x</td><td>$y</td></tr>\n";
}

function row2a($x, $y) {
    echo "<tr><td>$x</td><td width=50%>$y</td></tr>\n";
}

function row3($x, $y, $z) {
    echo "<tr><td width=30% valign=top align=right>$x</td><td>$y</td><td>$z</td></tr>\n";
}

function random_string() {
    return md5(uniqid(rand()));
}

function print_country_select($country) {
    $x = posix_getcwd();
    PassThru("$x/country_select '$country'");
}

function print_login_form_aux($next_url) {
    echo "<form method=post action=login_action.php>
        <input type=hidden name=next_url value='$next_url'>
        <table cellpadding=8>
        <tr><td align=right>
        Your account key:
        </td><td>
        <input name=authenticator size=40>
        </td></tr>

        <tr><td align=right>
        <br>
        </td><td>
        <input type=submit value='Log in'>
        </td></tr>
        </table>
    ";
    echo "<font size=1>
        If you don't know your account key,
        <a href=get_passwd.php>click here</a>.
        </font>
    ";
}

function print_login_form() {
    page_head("Please log in");
    echo "
        <h3>Please log in</h3>
        This function requires that you log in.
    ";
    $next_url = $_SERVER[REQUEST_URI];
    print_login_form_aux($next_url);
    page_tail();
}

// look for an element in some XML text
//
function parse_element($xml, $tag) {
    $element = null;
    $x = strstr($xml, $tag);
    if ($x) {
        $y = substr($x, strlen($tag));
        $n = strpos($y, "<");
        if ($n) {
            $element = substr($y, 0, $n);
        }
    }
    return $element;
}

// look for a particular element in the config.xml file
//
function parse_config($tag) {
    $element = null;
    $fp = fopen("config.xml", "r");
    while (1) {
        $buf = fgets($fp, 1024);
        if ($buf == null) break;
        $element = parse_element($buf, $tag);
        if ($element) break;
    }
    fclose($fp);
    return $element;
}

// Call this if for dynamic pages
//
function no_cache() {
    header ("Expires: Mon, 26 Jul 1997 05:00:00 GMT");    // Date in the past
    header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); // always modified
    header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
    header ("Pragma: no-cache");                          // HTTP/1.0
}

// A few functions relating to email-address munging
// A "munged" email address is of the form @X_Y,
// where X is a valid email address and Y is a random string.
// When an email address hasn't been validated yet, it's munged.
// (Used during account creation and email address changes)

// a valid email address is of the form A@B.C
// where A, B, C are nonempty and don't contain @ or .
//
function is_valid_email_addr($addr) {
    $x = strstr($addr, "@");
    if (!$x) return false;
    if (strlen($x) == strlen($addr)) return false;
    $x = substr($x, 1);
    if (strstr($x, "@")) return false;
    $y = strstr($x, ".");
    if (!$y) return false;
    if (strlen($y) == strlen($x)) return false;
    if (strlen($y) == 1) return false;
    $y = substr($y, 1);
    if (strstr($y, ".")) return false;
    if (strlen($y) == 0) return false;
    return true;
}

function munge_email_addr($email, $string) {
    return "@$email_$string";
}

// if email_addr is of the form @X_Y, split out the X and return true.
// otherwise return false
//
function split_munged_email_addr($addr, $string, &$email) {
}

?>
