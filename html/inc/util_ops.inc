<?php

require_once("../inc/db_ops.inc");
require_once("../inc/util.inc");
require_once("../project/project.inc");

define("EMAIL_EXISTS", -1);
define("EMAIL_UPDATED", 1);
define("EMAIL_FAIL", -2);

define("LG_FONT", "<font size=+1>");
define("SM_FONT", "<font size=-1>");

define("TD", "<td bgcolor=#708090>");
define("TD2", "<td colspan=2 bgcolor=#708090>");
define("TD3", "<td colspan=3 bgcolor=#708090>");

define("TABLE", "<table cellpadding=8 border=1 width=100%>");
define("TABLE2", "<table width=580>");

define("TITLE_COLOR", " bgcolor=000000 ");
define("TITLE_FONT", " <font color=ffffff> ");
define("BODY_COLOR", " bgcolor=ffffff ");
define("NOLOGIN", "Not logged in. Click <a href=login.php>here</a> to login.\n");

function boinc_real_escape_string($unstripped) {
  if (1) {
    // valid for PHP>=4.3.0, 5.  If there is a run-time php_version() function please use it
    // instead of this hack!
    return mysql_real_escape_string($unstripped);
  } else {
    return str_replace("\'", "'", str_replace("\\\"", "'", $unstripped));
  }
}

function admin_page_head($title) {
  echo "<head><title>$title</title><body " . BODY_COLOR . ">\n";
  echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=6><b><a href=index.php>".PROJECT.":</a>  $title</b></font></td></tr></table>\n";
}

function admin_page_tail() {
    echo "<hr><a href=index.php> Main admin page </a>\n";
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
    echo "<tr><td valign=top align=right>$x</td>\n<td>$y</td>\n</tr>\n";
}

function c_row2($color, $x, $y) {
    echo "<tr bgcolor=$color><td align=right>$x</td><td>$y</td></tr>\n";
}

?>
