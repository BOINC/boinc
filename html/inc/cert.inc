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

function credit_to_ops($credit, &$ops, &$unit) {
    // 100 units of credit is 86400*10^9 ops
    // 1 unit of credit is 864*10^9 ops
    $ops_trillion = ($credit*864)/1000;
    $unit = "trillion";
    $ops = $ops_trillion;
    if ($ops_trillion > 1000) {
        $ops = $ops_trillion/1000;
        $unit = "quadrillion";
    }
    if ($ops_trillion > 1000000) {
        $ops = $ops_trillion/1000000;
        $unit = "quintillion";
    }
    if ($ops_trillion > 1000000000) {
        $ops = $ops_trillion/1000000000;
        $unit = "sextillion";
    }
    $ops = number_format($ops, 2);
}

function credit_string($credit, $bolden) {
    $cobbs = number_format($credit, 0);

    credit_to_ops($credit, $ops, $unit);
      
    if ($bolden) {
        $lbold="[[";
        $rbold="]]";
    } else {
        $lbold="";        $rbold="";
    } 
    return " $lbold$cobbs Cobblestones$rbold of computation ($ops $unit floating-point operations)";
}

?>
