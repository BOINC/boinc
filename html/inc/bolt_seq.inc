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

class BoltSequence extends BoltSet {
    function __construct($name, $units, $attrs) {
        parent::__construct($name, $units, count($units), $attrs);
    }

    function order() {
        $this->ordered = true;
    }

    function restart(&$iter) {
        $state_rec = $iter->state[$this->name];
        if (!$state_rec) $state_rec = $this->init();
        $state_rec['nshown'] = 0;
        $state_rec['index'] = 0;
        $state_rec['child_name'] = null;
        $iter->state[$this->name] = $state_rec;
    }
}

function sequence() {
    $args = func_get_args();
    $units = array();
    $name = "";
	$attrs = null;
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
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
    return new BoltSequence($name, $units, $attrs);
}

?>
