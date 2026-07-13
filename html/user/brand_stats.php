<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// show statistics by client "brand"
 DEPRECATED

require_once("../inc/util.inc");

function show_form() {
    page_head("Show statistics by client brand");
    echo "
        <form action=brand_stats.php>
        Select brand:
        <p>
        <input type=radio name=brand value=1 checked>HTC Power to Give
        <p>
        <input type=submit value=\"Show stats\">
    ";
    page_tail();
}

function show_stats($brand) {
    switch ($brand) {
    case 1:
        $x = "HTC Power to Give";
        break;
    default:
        error_page("invalid brand");
    }
    $hosts = BoincHost::enum("os_name='Android' and serialnum like '%$x%'");
    $n = 0;
    $t = 0;
    $a = 0;
    foreach($hosts as $h) {
        $t += $h->total_credit;
        $a += $h->expavg_credit;
        if ($h->expavg_credit > .1) $n++;
    }
    page_head("Stats for $x");
    start_table();
    row2("Active devices", $n);
    row2("Average daily credit", $a);
    row2("Total credit", $t);
    end_table();
    page_tail();
}

$brand = get_int("brand", true);
if ($brand) {
    show_stats($brand);
} else {
    show_form();
}

?>
