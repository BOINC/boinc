<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

require_once("../inc/db_ops.inc");
require_once("../inc/util.inc");
require_once("../project/project.inc");

// TODO: get rid of the following.  use style sheets
define("TD", "<td bgcolor=\"#708090\">");
define("TD2", "<td colspan=\"2\" bgcolor=\"#708090\">");
define("TD3", "<td colspan=\"3\" bgcolor=\"#708090\">");
define("TABLE", "<table cellpadding=\"8\" border=\"1\" width=\"100%\">");
define("TABLE2", "<table width=\"580\">");

function admin_page_head($title) {
    $stylesheet = URL_BASE.STYLESHEET;

    echo "<html><head><title>$title</title>
        <link rel=stylesheet type=\"text/css\" href=\"".URL_BASE."main.css\" media=\"all\" />
        <link rel=\"stylesheet\" type=\"text/css\" href=\"$stylesheet\">
        </head>
        <body bgcolor=\"#ffffff\">
        <h2>".PROJECT.": $title</h2>
    ";
}

function admin_page_tail() {
    echo "<br><hr noshade size=1><center>
        <a href=\"index.php\"> Back to Main admin page </a></center>
        </body></html>
    ";
}

// TODO: get rid of all the following

function print_checkbox($text,$name,$checked) {
    echo "<input type=\"checkbox\" name=\"$name\""
        . (strlen($checked) ? " checked=\"checked\"" : "") . ">"
        . "$text\n"
        . "<p>\n";
}

function print_radio_button($text,$name,$value,$checked) {
    echo "<input type=\"radio\" name=\"$name\" value=\"$value\""
        . (strlen($checked) ? " checked=\"checked\"" : "") . ">"
        . "$text\n"
        . "<br>\n";
}

function print_text_field($text,$name,$value) {
    echo "$text <input type=\"text\" size=\"10\" name=\"$name\" value=\"$value\">\n"
         . "<p>\n";
}

function row($x, $y) {
    echo "<tr><td valign=\"top\" align=\"right\">$x</td>\n<td>$y</td>\n</tr>\n";
}

function c_row2($color, $x, $y) {
    echo "<tr bgcolor=\"$color\"><td align=\"right\">$x</td><td>$y</td></tr>\n";
}

function show_profile_link_ops($user) {
    if ($user->has_profile) { 
        row2("Profile",
            "<a href=\"".URL_BASE."view_profile.php?userid=$user->id\">View</a>"
        );
    }
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
