<?php

require_once("db.inc");

define("LG_FONT", "<font size=+1>");
define("SM_FONT", "<font size=-1>");
define("TD2", "<td colspan=2 bgcolor=#708090>");
define("TD3", "<td colspan=3 bgcolor=#708090>");
define("TABLE", "<table cellpadding=10 cellspacing=4 border=0 width=100%>");
define("TABLE2", "<table width=580>");
define("BG_COLOR", " bgcolor=cccccc ");
define("TITLE_COLOR", " bgcolor=000000 ");
define("TITLE_FONT", " <font color=ffffff> ");
define("BODY_COLOR", " bgcolor=ffffff ");
define("PROJECT_NAME", "Sample Project");

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
        printf("<br><a href=this_user.php>My %s</a>\n", PROJECT_NAME);
    } else {
        printf("Not logged in.  <a href=login.php>Log in.</a>\n");
    }
}

function page_head($title) {
    echo "<head><title>$title</title><body " . BG_COLOR . ">\n";
    echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=6><b><a href=index.php>".PROJECT_NAME.":</a>  $title</b></font></td></tr></table>\n";
}

function page_tail() {
    echo "<hr><table width=100%><tr><td align=center>|<a href=create_account.php> Create New Account </a>|<a href=login.php> Login </a>\n";
    echo "|<a href=home.php> User Page </a>|<a href=team.php> Teams </a>|<a href=index.html> Main Project Page </a>|</td></tr></table>\n";
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

function row3($x, $y, $z) {
    echo "<tr><td width=30% valign=top align=right>$x</td><td>$y</td><td>$x</td></tr>\n";
}

function random_string() {
    return md5(uniqid(rand()));
}

function print_country_select() {
    PassThru("/disks/milkyway/a/users/anderson/boinc/tools/country_select");
}

function print_page_header($title) {
    echo "<html>\n";
    echo "<head>\n";
    echo "<title>$title</title>\n";
    echo "</head>\n";
    echo "<body BGCOLOR=000000 TEXT=FFFFFF LINK=cc99ff VLINK=cc99ff>\n";
    echo "<br clear=all><br>\n";
}

function print_page_end() {
    echo "</body></html>";
}

?>