<?php

require_once("db.inc");

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

define("BG_COLOR", " bgcolor=cccccc ");
define("TITLE_COLOR", " bgcolor=000000 ");
define("TITLE_FONT", " <font color=ffffff> ");
define("BODY_COLOR", " bgcolor=ffffff ");
define("NOLOGIN", "Not logged in. Click <a href=login.php>here</a> to login.\n");
define("BADPASS", "The password you entered is incorrect. Click the <b>Back</b> button on your browser to re-enter your password or try again later.");
define("DIFFPASS", "You've typed two different passwords. Click the <b>Back</b> button on your browser to edit your information, making sure you type the same password in both password fields."); 

function get_project_name() {
    $fp = fopen(db_name, "r");
    $db_name = fgets($fp, 1024);
    $db_name = rtrim($db_name);
    return $db_name;
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
        printf("Logged in as %s.\n", $user->name);
        printf("<br><a href=login.php>Log in as someone else.</a>\n");
        $project = get_project_name();
        printf("<br><a href=this_user.php>My %s</a>\n", $project);
    } else {
	echo NOLOGIN;
    }
}

function page_head($title) {
    echo "<head><title>$title</title><body " . BG_COLOR . ">\n";
    $project = get_project_name();
    echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=6><b><a href=index.php>".$project.":</a>  $title</b></font></td></tr></table>\n";
}

function page_tail() {
    echo "<hr><table width=100%><tr><td align=center>|<a href=create_account.php> Create New Account </a>|<a href=login.php> Login </a>\n";
    echo "|<a href=home.php> User Page </a>|<a href=team.php> Teams </a>|<a href=index.php> Main Project Page </a>|</td></tr></table>\n";
}

function append_sql_query($original,$addition,$first) {
    if ($first == 1) {
        return $original . " where " . $addition;
    } else {
        return $original . " and " . $addition;
    }
}

function date_str($when) {
    return date("g:i A, l M j", $when);
}

function time_str($x) {
    if ($x == 0) return "---";
    return strftime("%T %b %e, %Y", $x);
}

function start_table() {
    echo "<table border=2 cellpadding=4 width=700>\n";
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

function print_submit($text,$name) {
    echo "<input type=submit name=\"$name\" value=\"$text\">\n";
}

function row($x, $y) {
    echo "<tr>\n<td width=30% colspan=2 valign=top align=right>$x</td>\n<td colspan=2>$y</td>\n</tr>\n";
}

function row2($x, $y) {
    echo "<tr><td>$x</td><td>$y</td></tr>\n";
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
    PassThru("/disks/milkyway/a/users/anderson/boinc/tools/country_select");
}

function print_page_header($title) {
    echo "<html>\n";
    echo "<head>\n";
    echo "<title>$title</title>\n";
    echo "</head>\n";
    echo "<body TEXT=000000 BGCOLOR=FFFFFF>\n";
    echo "<h2>$title</h2>\n";
    echo "<a href=index.html>Return to Main Page</a>\n";
    echo "<br clear=all>\n";
}

function print_page_end() {
    echo "</body></html>";
}

?>
