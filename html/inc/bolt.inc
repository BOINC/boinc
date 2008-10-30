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

// Bolt course document API

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

abstract class BoltUnit {
    public $name;
        // Logical name.  Changing this makes it a different unit.
        // For items, this is the filename with query string;
        // for structures, it must be specified with name()
    public $title;
        // Optional; used when showing course history outline.
    public $is_item;
	public $attrs;		// course-defined

    abstract function walk(&$iter, $incr, &$frac_done);
        // multi-purpose function for traversing a course.
        // Create entry in $iter->state if not there.
        // Recurse to first child.
        // If first child is an item, set $iter->item
        // If incr is set
        //     the bottom-level non-item unit should increment.
        //     return value: true if the caller should increment
        // frac_done: Fraction done (of this unit and any subunits)
}

// base class for exercise and lesson
//
class BoltItem extends BoltUnit {
    public $filename;
    public $query_string;
    function __construct($filename, $title, $attrs) {
        $p = strpos($filename, '?');
        if ($p === false) {
            $this->filename = $filename;
            $this->query_string = null;
        } else {
            $this->filename = substr($filename, 0, $p);
            $this->query_string = substr($filename, $p+1);
        }
        $this->name = $filename;
        $this->title = $title;
        $this->is_item = true;
		$this->attrs = $attrs;
    }
    function begin() {
        return array(new BoltFrame($this));
    }
    function walk(&$iter, $incr, &$frac_done) {
        echo "SHOULDN'T BE HERE\n";
    }
}

class BoltLesson extends BoltItem {
    function is_exercise() {
        return false;
    }
}

class BoltExercise extends BoltItem {
    public $callback;
        // called as func($student, $score, $query_string) after scoring
    public $weight;
    public $has_answer_page;

    function __construct(
        $filename, $title, $attrs, $callback, $weight, $has_answer_page
    ) {
        parent::__construct($filename, $title, $attrs);
        $this->callback = $callback;
        $this->weight = $weight;
        $this->has_answer_page = $has_answer_page;
    }
    function is_exercise() {
        return true;
    }
}

// Base class for control structures (all units other than items).
// The state of a control structure has two parts:
// 1) a transient PHP object
// 2) a persistent "state record" (stored in JSON in the DB)
//
// The PHP object has the following properties:
// - a set of units
// - ordered: a flag for whether the set has been ordered yet
// - order($state_rec): a function for ordering this set,
//   defined in the derived class
//   (i.e., random, student-specific, or identity)
//   This orders the set, sets "ordered", and adds info to the state rec
//   saying how the ordering was done (e.g. RNG seed)
// - a number "ntoshow" for how many units to show
//
// The state record has the following items:
// - index: index into the unit array
// - nshown: for how many units completed so far
// - child_name: name of current child, or null
//
class BoltSet extends BoltUnit {
    public $units;
    function __construct($name, $units, $ntoshow, $attrs) {
        $this->name = $name;
        $this->is_item = false;
        $this->units = $units;
        $this->ntoshow = $ntoshow;
        $this->ordered = false;
		$this->attrs = $attrs;
    }

    // restart this unit - set its state record to an initial state
    //
    function restart(&$iter) {
        $state_rec = $iter->state[$this->name];
        if (!$state_rec) $state_rec = $this->init();
        $state_rec['nshown'] = 0;
        $state_rec['child_name'] = null;
        $iter->state[$this->name] = $state_rec;
    }

    // initialize this unit (once per course)
    //
    function init(&$iter) {
        $state_rec = array();
        $state_rec['index'] = 0;
        $iter->state[$this->name] = $state_rec;
        return $state_rec;
    }

    function finished(&$iter) {
        $this->restart($iter);
    }

    function walk(&$iter, $incr, &$frac_done) {
        $n = count($this->units);
        if (array_key_exists($this->name, $iter->state)) {
            $state_rec = $iter->state[$this->name];
            $child_name = $state_rec['child_name'];
            $nshown = $state_rec['nshown'];
            if (!$this->ordered) {
                $this->order($iter);
            }

            // look up unit by name
            // 
            $child = null;
            for ($i=0; $i<$n; $i++) {
                $c = $this->units[$i];
                if ($c->name == $child_name) {
                    $child = $c;
                    break;
                }
            }

            // if not there, look up by index
            //
            if (!$child) {
                $i = $state_rec['index'];
                if ($i >= $n) {
                    // and if index is too big, use last unit
                    //
                    $i = $n-1;
                }
                $child = $this->units[$i];
            }

            // at this point, $child is the current unit, and $i is its index
            //
            if ($incr) {
                if ($child->is_item) {
                    $my_inc = true;
                } else {
                    $my_inc = $child->walk($iter, $incr, $frac_done);
                }
                if ($my_inc) {
                    $nshown++;
                    if ($nshown == $this->ntoshow) {
                        $frac_done = 1;
                        $this->finished($iter);
                        return true;
                    } else {
                        $i = ($i+1)%$n;
                    }
                }
            }
        } else {
            // here if no state record; initialize
            //
            $i = 0;
            $nshown = 0;
            $this->init($iter);
            $this->order($iter);
        }

        // at this point, $i is index of current child, $nshown is valid,
        // and this unit has a record in the state array
        //
        $child = $this->units[$i];
        $frac_done = $nshown/$n;
        $state_rec = $iter->state[$this->name];
        $state_rec['index'] = $i;
        $state_rec['nshown'] = $nshown;
        $state_rec['child_name'] = $child->name;
        $iter->state[$this->name] = $state_rec;
        if ($child->is_item) {
            $iter->item = $child;
        } else {
            $child->walk($iter, false, $f);
            $frac_done += $f*(1/$n);
        }
        return false;
    }

    // return the name of our child, if we exist in the state
    //
    function get_child($state) {
        if (array_key_exists($this->name, $state)) {
            $state_rec = $state[$this->name];
            $child_name = $state_rec['child_name'];
            foreach($this->units as $c) {
                if ($c->name == $child_name) {
                    return $c;
                }
            }
        }
        return null;
    }

}

function name($n) {
    return array('name', $n);
}

function title($n) {
    return array('title', $n);
}

function number($n) {
    return array('number', $n);
}

function filename($n) {
    return array('filename', $n);
}

function has_answer_page($n) {
    return array('has_answer_page', $n);
}

function attrs($n) {
    return array('attrs', $n);
}

function callback($n) {
    return array('callback', $n);
}

function lesson() {
    $filename = "";
    $title = "";
	$attrs = null;

    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'title': $title = $arg[1]; break;
            case 'filename': $filename = $arg[1]; break;
            case 'attrs': $attrs = $arg[1]; break;
            default: echo "Unrecognized lesson parameter: ", $arg[0], "\n"; break;
            }
        } else {
            echo "unprocessed arg of class ".get_class($arg);
        }
    }
    if (!$title) {
        $title = $filename;
    }
    if (!$filename) {
        error_page("Missing filename in lesson");
    }
    return new BoltLesson($filename, $title, $attrs);
}

function exercise() {
    $filename = "";
    $title = "";
	$attrs = null;
    $weight = 1;
    $has_answer_page = true;

    $args = func_get_args();
    $callback = null;
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'title': $title = $arg[1]; break;
            case 'filename': $filename = $arg[1]; break;
            case 'attrs': $attrs = $arg[1]; break;
            case 'callback': $callback = $arg[1]; break;
            case 'weight': $weight = $arg[1]; break;
            case 'has_answer_page': $has_answer_page = $arg[1]; break;
            default: echo "Unrecognized exercise parameter: ", $arg[0], "\n"; break;
            }
        }
    }
    if (!$title) {
        $title = $filename;
    }
    if (!$filename) {
        error_page("Missing filename in lesson");
    }
    return new BoltExercise(
        $filename, $title, $attrs, $callback, $weight, $has_answer_page
    );
}

function item_attrs() {
    global $item;
    return $item->attrs;
}

function student_sex() {
    global $user;
    return $user->bolt->sex;
}

function student_age() {
    global $user;
    if (!$user->bolt->birth_year) return -1;
    $date = getdate();
    $this_year = $date["year"];
    return $this_year - $user->bolt->birth_year;
}

function student_country() {
    global $user;
    return $user->country;
}

function student_name() {
    global $user;
    return $user->name;
}

function student_attrs() {
    global $user;
    return unserialize($user->bolt->attrs);
}

function set_student_attrs($attrs) {
    global $user;
    $attrs = serialize($attrs);
    $user->bolt->update("attrs='$attrs'");
}

require_once('../inc/bolt_seq.inc');
require_once('../inc/bolt_rnd.inc');
require_once('../inc/bolt_xset.inc');
require_once('../inc/bolt_select.inc');

?>
