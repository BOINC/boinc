<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// show certificate listing join time and credit across all projects
// Projects can customize this:
// https://github.com/BOINC/boinc/wiki/WebConfig#certificate-related-constants

require_once("../inc/util.inc");
require_once("../inc/cert.inc");
require_once("../inc/user.inc");

$border = get_str("border", true);
if ($border=="no") {
    $border=0;
} else {
    $border=8;
}

// Make sure user_id is in the URL so that share functions work
//
$user_id = get_int('user_id', true);
if (!$user_id) {
    $user = get_logged_in_user();
    Header(sprintf('Location: %s/cert_all.php?user_id=%d%s',
        url_base(), $user->id, $border==0?'&border=no':''
    ));
    exit;
}
$user = BoincUser::lookup_id($user_id);

$join = gmdate('j F Y', $user->create_time);
$today = gmdate('j F Y', time());

$font = "\"Optima,Lucida Bright,Times New Roman\"";

$user = get_other_projects($user);

if (!$user->projects) {
    error_page("No accounts found");
}

$total_credit = 0;
foreach ($user->projects as $p) {
    $total_credit += $p->total_credit;
}

$credit = credit_string($total_credit, false);

function show_proj($p) {
    $join = date('j F Y', $p->create_time);
    echo "<tr> <td>$p->name</td><td> $p->total_credit</td><td>$join</td></tr>
    ";
}

echo "
    <table id=\"certificate\" width=900 height=650 border=$border cellpadding=20><tr><td>
    <center>
    <table width=700 border=0><tr><td style=\"background-position:center; background-repeat:no-repeat\" background=https://boinc.berkeley.edu/logo/boinc_fade_600.png>
    <center>
    <font style=\"font-size: 52\" face=$font>Certificate of Computation

    <font face=$font style=\"font-size:28\">
    <br><br>
    This certifies that
    <p>
    <font face=$font style=\"font-size:32\">
    $user->name

    <font face=$font style=\"font-size:18\">
    <p>
    has contributed $credit
    to the following scientific research projects:

    <center>
    <table width=80%>
    <tr><th align=left>Project</th><th align=left>Cobblestones</th><th align=left>Joined</th></tr>
";
foreach ($user->projects as $p) {
    if ($p->total_credit<100) continue;
    show_proj($p);
}
echo "
    </table>
    </center>
";

echo "
    </td>
";
echo "
</td><tr></table></table>
";
show_download_button();
show_share_buttons();
?>
