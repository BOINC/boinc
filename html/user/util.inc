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
define("TABLE2", "<table width=580>");

define("BG_COLOR", " bgcolor=ffffff ");
define("TITLE_COLOR", " bgcolor=000000 ");
define("TITLE_FONT", " <font color=ffffff> ");
define("BODY_COLOR", " bgcolor=ffffff ");
define("NOLOGIN", "Not logged in. Click <a href=login.php>here</a> to login.\n");
define("BADPASS", "The password you entered is incorrect. Click the <b>Back</b> button on your browser to re-enter your password or try again later.");
define("DIFFPASS", "You've typed two different passwords. Click the <b>Back</b> button on your browser to edit your information, making sure you type the same password in both password fields."); 
define("PROJECT", "Sample project");


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
        printf("Logged in as %s.\n", $user->name);
        printf("<br><a href=login.php>Log in as someone else.</a>\n");
    } else {
	echo NOLOGIN;
    }
}

function page_head($title) {
    echo "<head><title>$title</title><body ".BG_COLOR.">\n";

    // Put your project title and logo here

    echo "<h2>Sample Project</h2>\n";
    echo "<h3>$title</h3>\n";
}

function page_tail() {

    // put your copyright notice etc. here

    echo "<hr>Copyright (c) 2002 Sample Project\n";
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

function print_country_select() {
    $x = posix_getcwd();
    PassThru("$x/country_select");
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

?>
