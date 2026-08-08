<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// show a (textual) histogram of job sizes in CPU seconds.

require_once('../inc/boinc_db.inc');
require_once('../inc/common_defs.inc');

function main() {
    $ress = BoincResult::enum_fields(
        'cpu_time, hostid',
        sprintf('outcome=%d and cpu_time>0 order by id desc limit 1000', RESULT_OUTCOME_SUCCESS)
    );
    $hosts = [];
    $samples = [];
    foreach ($ress as $res) {
        if (array_key_exists($res->hostid, $hosts)) {
            $host = $hosts[$res->hostid];
        } else {
            $host = BoincHost::lookup_id($res->hostid);
            $hosts[$res->hostid] = $host;
        }
        $fpops = $res->cpu_time * $host->p_fpops;
        $samples[] = $res->cpu_time;
        //$fpops /= 1e9;
        //echo "$res->cpu_time $fpops\n";
    }

    show($samples);
}

function show($samples) {
    $x = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
    foreach ($samples as $s) {
        $x[(int)log10($s)] += 1;
    }
    for ($i=0; $i<16; $i++) {
        echo sprintf("%d: %d\n", $i, $x[$i]);
    }
}

main();

?>
