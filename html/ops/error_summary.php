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

// generate a list of the most common exit codes
// and links to N examples of each

require_once("../inc/util.inc");
require_once("../inc/result.inc");

$ncodes = get_int('ncodes', true);
if (!$ncodes) $ncodes = 10;
$nresults_per_code = get_int('nresults_per_code', true);
if (!$nresults_per_code) $nresults_per_code = 10;

function compare ($x, $y) {
    return $x->count < $y->count;
}

$t = time() - 7*86400;
$results = BoincResult::enum_fields("id, exit_status", "server_state=5 and outcome=3 and exit_status<>0 and received_time>$t", "");
$error_codes = array();
foreach ($results as $r) {
    $e = $r->exit_status;
    if (array_key_exists($e, $error_codes)) {
        $x = $error_codes[$e];
        $x->count++;
        $x->results[] = $r;
        $error_codes[$e] = $x;
    } else {
        $x = new StdClass;
        $x->count = 1;
        $x->results = array($r);
        $error_codes[$e] = $x;
    }
}

uasort($error_codes, 'compare');

page_head("Error summary");
$i = 0;
foreach ($error_codes as $code => $x) {
    if ($i++ >= $ncodes) break;
    echo "<h2>Exit status: ".exit_status_string($code)." ($x->count results)</h2>\n";
    $results = $x->results;
    $j = 0;
    foreach ($results as $r) {
        if ($j++ >= $nresults_per_code) break;
        echo "&nbsp;<a href=".url_base()."result.php?resultid=$r->id>$r->id</a><br>\n";
    }
}
page_tail();

?>
