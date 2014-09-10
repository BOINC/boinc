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

// express a time difference in readable form, e.g. "7 days ago".
// If it's more than 30 days, just show the date
//
function time_diff_str($t1, $t2) {
    if (!$t1 || !$t2) return "---";
    $diff = $t2 - $t1;
    if ($diff<0){
        $pre="In ";
        $post="";
        $diff=-$diff;
    } else {
        if ($diff > 86400*30) {
            return date_str($t1);
        }
        $pre="";
        $post=" ago";
    }
    $x = "";

    if ($diff >= 3600*24) {
        $n = (int) ($diff/(3600*24));
        if ($n == 1) {
            $x .= "1 day ";
        } else {
            $x .= "$n days ";
        }
        return $pre.$x.$post;
        //$diff -= $n*3600*24;
    } elseif ($diff >= 3600) {
        $n = (int) ($diff/3600);
        if ($n == 1) {
            $x .= "1 hour ";
        } else {
            $x .= $n." hours ";
        }
        return $pre.$x.$post;
        //$diff -= $n*3600;
    } elseif ($diff >= 60) {
        $n = (int) ($diff/60);
        if ($n == 1) {
            $x .= "1 minute ";
        } else {
            $x .= $n." minutes ";
        }
        return $pre.$x.$post;
    } elseif($diff > 1 || $diff==0) {
        return $pre."$diff seconds".$post;
    } elseif($diff == 1){
        return $pre."$diff seconds".$post;
    }
}
?>
