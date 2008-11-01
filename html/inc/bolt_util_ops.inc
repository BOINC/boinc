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


// Utility functions for admin pages

require_once("../inc/util_ops.inc");

// get names of units of a given type

function units_of_type($unit, $type) {
    $names = array();
    if (get_class($unit) == $type) {
        $names[] = $unit->name;
    }
    if (is_subclass_of($unit, "BoltSet")) {
        foreach ($unit->units as $u) {
            $n = units_of_type($u, $type);
            $names = array_merge($names, $n);
        }
    }
    return array_unique($names);
}

// show a menu of select units
//
function choose_select($top_unit) {
    echo "<select name=select_name>
        <option selected> ---
    ";
    $names = units_of_type($top_unit, "BoltSelect");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

// show a menu of exercise sets
//
function choose_xset($top_unit) {
    echo "<select name=xset_name>
        <option selected> ---
    ";
    $names = units_of_type($top_unit, "BoltExerciseSet");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

// Find a unit of given name
//
function lookup_unit($top_unit, $name) {
    if ($top_unit->name == $name) return $top_unit;
    if (is_subclass_of($top_unit, "BoltSet")) {
        foreach ($top_unit->units as $child) {
            $u = lookup_unit($child, $name);
            if ($u) return $u;
        }
    }
    return null;
}

////// Statistics

// compute the mean and stdev of an array
//
function mean_stdev($array, &$mean, &$stdev) {
    $n = 0;
    $m = 0;
    $m2 = 0;

    foreach ($array as $x) {
        $n++;
        $delta = $x - $m;
        $m += $delta/$n;
        $m2 += $delta*($x-$m);
    }
    $mean = $m;
    $stdev = sqrt($m2/($n-1));
}

// approximate the 90% confidence interval for the mean of an array
//
function conf_int_90($array, &$lo, &$hi) {
    $n = count($array);
    mean_stdev($array, $mean, $stdev);

    // I'm too lazy to compute the t distribution
    $t_90 = 1.7;
    $d = $t_90 * $stdev / sqrt($n);
    $lo = $mean - $d;
    $hi = $mean + $d;
}

function test_stats() {
    $a = array(1,1,1,1,0,1,1,1,3, 1, 1, 1, 1);
    mean_stdev($a, $mean, $stdev);
    echo "mean: $mean stdev: $stdev\n";
    conf_int_90($a, $lo, $hi);
    echo "lo $lo hi $hi\n";
}

//////////// graph drawing

function compare_bar($title, $n, $width, $lo, $hi) {
    $x1 = $width*$lo;
    $x2 = $width*($hi-$lo);
    $a1 = number_format($lo*100);
    $a2 = number_format($hi*100);
    return "
        <tr>
        <td>$title<br><span class=note>($n students)</span></td>
        <td>
            <table class=bolt_bar width=$width><tr class=bolt_bar>
            <td class=bolt_bar1 width=$x1 bgcolor=lightgray align=right>$a1</td>
            <td class=bolt_bar2 width=$x2 bgcolor=black></td>
            <td class=bolt_bar1 bgcolor=lightgray>$a2</td>
            </tr></table>
        </td>
        </tr>
    ";
}

function compare_bar_insuff($title, $width) {
    return "
        <tr>
        <td>$title</td>
        <td>
            <table class=bolt_bar width=$width><tr class=bolt_bar>
            <td class=bolt_bar1 bgcolor=lightgray>Insufficient data</td>
            </tr></table>
        </td>
        </tr>
    ";
}

function outcome_graph($x, $width) {
    $n = $x[0]+$x[1]+$x[2];
    if (!$n) return empty_cell();
    $x0 = $width*$x[1]/$n;
    $x1 = $width*$x[0]/$n;
    $x2 = $width*$x[2]/$n;
    if ($x[1]/$n>0.05) {
        $t0 = number_format(100*$x[1]/$n)."%";
    } else {
        $t0 = "";
    }
    $s = "<td>
        <table class=bolt_bar><tr>
    ";
    if ($x0) {
        $s .= "<td class=bolt_bar height=10 width=$x0 bgcolor=#88ff88><span class=note>$t0</span></td>
        ";
    }
    if ($x1) {
        $s .= "<td class=bolt_bar height=10 width=$x1 bgcolor=#ff8888><br></td>
        ";
    }
    if ($x2) {
        $s .= "<td class=bolt_bar height=10 width=$x2 bgcolor=#ffff88><br></td>
        ";
    }
    $s .= "</tr></table>
        </td>
    ";
    return $s;
}

function time_graph($t, $w) {
    if ($t == 0) return "<td>---</td>";
    $x = (log10($t)+2)*$w/4;
    $t = number_format($t, 1);
    return "<td>
        <table class=bolt_bar><tr>
            <td class=bolt_bar height=10 width=$x bgcolor=#ffffcc><span class=note>$t sec</span></td>
        </tr></table>
    </td>";
}

function score_graph($t, $w) {
    if ($t == 0) return "<td>---</td>";
    $x = $t*$w;
    $y = (1-$t)*$w;
    $t = number_format($t*100);
    $s = "<td>
        <table class=bolt_bar><tr>
    ";
    if ($x) {
        $s .= "<td class=bolt_bar height=10 width=$x bgcolor=#ffffcc><span class=note>$t%</span></td>
        ";
    }
    if ($y) {
        $s .= "<td class=bolt_bar width=$y bgcolor=gray></td>
        ";
    }
    $s .= "</tr></table></td>
    ";
    return $s;
}

function empty_cell() {
    return "<td><br></td>";
}

function empty_row() {
    return "<tr><td><br></td></tr>";
}

function bolt_style() {
    echo "
        <link rel=\"stylesheet\" type=\"text/css\" href=\"".URL_BASE."bolt.css\">
    ";
}
?>
