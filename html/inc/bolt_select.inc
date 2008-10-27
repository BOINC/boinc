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

function select_cmp($a, $b) {
    if ($a->value == $b->value) return 0;
    return ($a->value < $b->value)?1:-1;
}

class BoltSelect extends BoltSet {
    public $valuator;

    function __construct($name, $units, $valuator, $attrs) {
        $this->valuator = $valuator;
        parent::__construct($name, $units, 1, $attrs);
    }

    function order() {
        global $student;
        foreach ($this->units as $u) {
            $func = $this->valuator;
            $u->value = $func($student, $u);
        }
        usort($this->units, 'select_cmp');
        $this->ordered = true;
    }

    function finished($iter) {
        global $user;
        global $course;
        global $view;

        $state_rec = $iter->state[$this->name];
        $child_name = $state_rec['child_name'];
        $now = time();
        BoltSelectFinished::insert("(user_id, course_id, end_time, name, selected_unit, view_id) values ($user->id, $course->id, $now, '$this->name', '$child_name', $view->id)");
        parent::finished($iter);
    }
}

function valuator($n) {
    return array('valuator', $n);
}

function select() {
    $args = func_get_args();
    $units = array();
    $name = "";
	$attrs = null;
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'valuator': $valuator = $arg[1]; break;
            case 'attrs': $attrs = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        } else if (is_object($arg)) {
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else {
                echo "Unrecognized arg: ";
                print_r($arg);
            }
        }
    }
    if (!$valuator) {
        error_page("missing valuator");
    }
    return new BoltSelect($name, $units, $valuator, $attrs);
}

?>
