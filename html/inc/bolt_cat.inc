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

// represents a categorization of students
//
abstract class Categorization {
    abstract function name();
        // returns descriptive name
    abstract function categories();
        // returns list of categories
    abstract function categorize($user);
        // returns a student's category
}

class CatSex extends Categorization {
    function name() {
        return "Sex";
    }
    function categories() {
        return array ("Male", "Female", "Unknown");
    }
    function categorize($user) {
        switch ($user->bolt->sex) {
        case 1: return "Male";
        case 2: return "Female";
        default: return "Unknown";
        }
    }
}

$x = localtime(time(), true);
$this_year = 1900 + $x['tm_year'];

class CatAge20 extends Categorization {
    function name() {
        return "Age (20-year groups)";
    }
    function categories() {
        return array("0-19", "20-39", "40-59", "60-79", "80+", "Unknown");
    }
    function categorize($user) {
        if (!$user->bolt->birth_year) return "Unknown";
        global $this_year;
        $n = $this_year - $user->bolt->birth_year;
        if ($n < 20) return "0-19";
        if ($n < 40) return "20-39";
        if ($n < 60) return "40-59";
        if ($n < 80) return "60-79";
        return "80+";
    }
}

$categorizations = array(new CatSex(), new CatAge20());

function lookup_categorization($name) {
    global $categorizations;
    foreach ($categorizations as $c) {
        if ($c->name() == $name) return $c;
    }
    return null;
}

function filter_form($sel_name, $sel_cat) {
    global $categorizations;
    $checked = (!$sel_name || $sel_name == "none")?"checked":"";
    echo "
        Filter by:
        <ul>
        <li><input type=radio name=filter value=none $checked> None
    ";
    foreach ($categorizations as $c) {
        $name = $c->name();
        $cats = $c->categories();
        echo "
            <li> $name
            <ul>
        ";
        foreach ($cats as $x) {
            $checked = ($sel_name == $name && $sel_cat == $x) ? "checked":"";
            echo "
                <li> <input type=radio name=filter value=\"$name:$x\" $checked> $x
            ";
        }
        echo "</ul>";
    }
    echo "</ul>";
}

function breakdown_form($sel_name) {
    global $categorizations;
    $checked = (!$sel_name || $sel_name == "none")?"checked":"";
    echo "
        Break down by:
        <ul>
        <li><input type=radio name=breakdown value=none $checked> None
    ";
    foreach ($categorizations as $c) {
        $name = $c->name();
        $checked = ($sel_name == $name)?"checked":"";
        echo "
            <li> <input type=radio name=breakdown value=\"$name\" $checked> $name
        ";
    }
    echo "</ul>";
}

// return filter and breakdown info for URLs
//
function filter_url() {
    global $filter, $filter_cat, $breakdown, $breakdown_cat;

    $x = "";
    if ($filter && $filter_cat) {
        $x .= "&filter=".$filter->name.":$filter_cat";
    }
    if ($breakdown && $breakdown_cat) {
        $x .= "&breakdown=".$breakdown->name.":$breakdown_cat";
    }
    return $x;
}

// get filter and breakdown from form vars
//
function get_filters_from_form() {
    global $breakdown, $breakdown_cat, $filter, $filter_cat;

    $breakdown_cat = null;
    $breakdown_info = get_str('breakdown', true);
    if ($breakdown_info && $breakdown_info != 'none') {
        $arr = explode(":", $breakdown_info);
        $breakdown_name = $arr[0];
        if (count($arr) == 2) {
            $breakdown_cat = $arr[1];
        }
        $breakdown = lookup_categorization($breakdown_name);
        if (!$breakdown) error_page("unknown breakdown $breakdown_name");
    } else {
        $breakdown = null;
    }
    $filter_info = get_str('filter', true);
    if ($filter_info && $filter_info != "none") {
        $arr = explode(":", $filter_info);
        $filter_name = $arr[0];
        $filter_cat = $arr[1];
        $filter = lookup_categorization($filter_name);
        if (!$filter) error_page("unknown filter $filter_name");
    } else {
        $filter_cat = "";
        $filter = null;
    }
}

?>
