<?php

require_once("db.inc");

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
define("TABLE2", "<table width=580>");

define("BG_COLOR", " bgcolor=ffffff ");
define("TITLE_COLOR", " bgcolor=000000 ");
define("TITLE_FONT", " <font color=ffffff> ");
define("BODY_COLOR", " bgcolor=ffffff ");
define("PROJECT", "Astropulse");
define("MASTER_URL", "http://maggie.ssl.berkeley.edu/ap/");

// Sends the authenticator to the given email address
//
function send_auth_email($email_addr, $auth) {
    mail($email_addr, PROJECT." new account confirmation",
"This email confirms the creation of your account with ".PROJECT.",
a distributed computing project based on BOINC.\n
The URL for this project:    ".MASTER_URL."
Your account key:            $auth\n
Copy and paste these into the BOINC client when prompted.\n
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

function get_user_from_cookie() {
    $auth = "";
    $c = getenv("HTTP_COOKIE");
    $d = str_replace("; ", "&", $c);
    parse_str($d);
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

function page_head($title) {
    echo "<head><title>$title</title><body ".BG_COLOR.">\n";

    // Put your project title and logo here

    echo "<h2>".PROJECT."</h2><hr>\n";
    echo "<h3>$title</h3>\n";
}

function page_tail() {

    echo "<br><hr><center><a href=index.php>Return to main ".PROJECT." page</a><br>\n";

    // put your copyright notice etc. here

    echo "<br><br>Copyright (c) 2002 ".PROJECT."</center>\n";
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

function print_login_form() {
    page_head("Please log in");
    echo "This function requires that you be logged in.\n";
    echo "Please <a href=login_form.php>log in</a>.\n<p>\n";
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

?>
