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

define("CREDIT_HALF_LIFE", 86400*7);

// update a credit average.
// this must match the function in lib/util.C

function update_average($now, $work_start_time, $work, &$avg, &$avg_time) {
    if ($avg_time) {
        $diff = $now - $avg_time;
        if ($diff <0) $diff = 0;
        $diff_days = $diff/86400;
        $weight = exp(-$diff*M_LN2/CREDIT_HALF_LIFE);
        $avg *= $weight;

        if ((1.0-$weight)>0.000001) {
            $avg += (1.0-$weight)*($work/$diff_days);
        }
        else {
            $avg += M_LN2*$work*86400/CREDIT_HALF_LIFE;
        }
    } else if ($work) {
        $dd = ($now - $work_start_time)/86400;
        $avg = $work/$dd;
    }
    $avg_time = $now;
}

?>
