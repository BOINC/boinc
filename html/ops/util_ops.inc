<?php

require_once("db_ops.inc");

define("EMAIL_EXISTS", -1);
define("EMAIL_UPDATED", 1);
define("EMAIL_FAIL", -2);

define("LG_FONT", "<font size=+1>");
define("SM_FONT", "<font size=-1>");

define("TD", "<td bgcolor=#708090>");
define("TD2", "<td colspan=2 bgcolor=#708090>");
define("TD3", "<td colspan=3 bgcolor=#708090>");

define("TABLE", "<table cellpadding=10 cellspacing=4 border=0 width=100%>");
define("TABLE2", "<table width=580>");

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
    echo "<head><title>$title</title><body " . BODY_COLOR . ">\n";
    echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=6><b><a href=index.php>".PROJECT.":</a>  $title</b></font></td></tr></table>\n";
}

function page_tail() {
    echo "<hr><a href=index.php> Main admin page </a>\n";
}

function date_str($x) {
    if ($x == 0) return "---";
    // return date("g:i A, l M j", $when);
    return strftime("%Y-%m-%d", $x);
}

function time_str($x) {
    if ($x == 0) return "---";
    // return strftime("%T %b %e, %Y", $x);
    return strftime("%Y-%m-%d %H:%M:%S", $x);
}

function start_table() {
    // echo "<table border=2 cellpadding=4 width=700>\n";
    echo "<table border=2 cellpadding=4 width=100%>\n";
}

function end_table() {
    echo "</table>\n";
}

function print_checkbox($text,$name,$checked) {
    echo "<input type=checkbox name=$name"
        . (strlen($checked) ? " checked" : "") . ">"
        . "$text\n"
        . "<p>\n";
}

function print_radio_button($text,$name,$value,$checked) {
    echo "<input type=radio name=$name value=$value"
        . (strlen($checked) ? " checked" : "") . ">"
        . "$text\n"
        . "<br>\n";
}

function print_text_field($text,$name,$value) {
    echo "$text <input type=text size=10 name=$name value=\"$value\">\n"
         . "<p>\n";
}

function row($x, $y) {
    echo "<tr>\n<td width=30% colspan=2 valign=top align=right>$x</td>\n<td colspan=2>$y</td>\n</tr>\n";
}

function row2($x, $y) {
    echo "<tr><td align=right>$x</td><td>$y</td></tr>\n";
}
function c_row2($color, $x, $y) {
    echo "<tr bgcolor=$color><td align=right>$x</td><td>$y</td></tr>\n";
}

function row3($x, $y, $z) {
    echo "<tr><td width=30% valign=top align=right>$x</td><td>$y</td><td>$z</td></tr>\n";
}

function row4($xx, $xy, $yx, $yy) {
    echo "<tr><td width=25% valign=top align=right>$xx</td><td width=25%>$xy</td>"
        . "<td width=25% align=right>$yx</td><td width=%25>$yy</td></tr>\n";
}

function random_string() {
    return md5(uniqid(rand()));
}

function print_country_select() {
    PassThru("country_select");
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
    $fp = fopen("../config.xml", "r");
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
