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

require_once("../inc/util.inc");
require_once("../inc/cert.inc");

$user = get_logged_in_user();
$team = BoincTeam::lookup_id($user->teamid);
if (!$team) error_page("no team");

$join = date('j F Y', $team->create_time);
$today = date('j F Y', time(0));

credit_to_ops($team->total_credit, $ops, $unit);

$border=$_GET["border"];
if ($border=="no") {
    $border = 0;
} else {
    $border=8;
}

$credit = credit_string($team->total_credit, false);

$title_font = "\"Optima,ZapfChancery\"";
$font = "\"Optima,Lucida Bright,Times New Roman\"";

echo "
    <table width=900 height=650 border=$border cellpadding=20><tr><td>
    <center>
    <table width=700 border=0><tr><td>
    <center>
    <font style=\"font-size: 52\" face=$title_font>Certificate of Computation


    <font face=$font style=\"font-size:28\">
    <br><br><br>
    This certifies that team
    <p>
    <font face=$font style=\"font-size:32\">
    $team->name

    <font face=$font style=\"font-size:18\">
    <p>
    has participated in ".PROJECT." since $join,
    and has contributed $credit
    to ".PROJECT.".

    <br><br><br>
    </td><tr></table>
    <table width=100%><tr>
    <td width=40><br></td>
    <td align=left>
    <font face=$font style=\"font-size:16\">
";
if (defined("CERT_SIGNATURE")) {
    echo "
        <img src=".CERT_SIGNATURE.">
        <br>
    ";
}
if (defined("CERT_DIRECTOR_NAME")) {
    echo CERT_DIRECTOR_NAME." <br>Director, ".PROJECT."
        <br>
    ";
}
echo "
    <br>
    $today
    </td>
";
if (defined("CERT_PROJECT_LOGO")) {
    echo "
        <td align=center valign=center> <img src=".CERT_PROJECT_LOGO."> </td>
    ";
}
if (defined("CERT_INSTITUTION_LOGO")) {
    echo "
        <td align=center width=30% valign=center><img src=".CERT_INSTITUTION_LOGO."></td>
    ";
}
echo "
</td><tr></table>
";
?>
