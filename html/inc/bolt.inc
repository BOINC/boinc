<?php

// Bolt course document API

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

abstract class BoltUnit {
    public $name;   // logical name.
    public $title;
    public $is_item;

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
    function __construct($name, $title, $filename) {
        $p = strpos($filename, '?');
        if ($p === false) {
            $this->filename = $filename;
            $this->query_string = null;
        } else {
            $this->filename = substr($filename, 0, $p);
            $this->query_string = substr($filename, $p+1);
        }
        $this->name = $name;
        $this->title = $title;
        $this->is_item = true;
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
    function __construct($name, $units, $ntoshow) {
        $this->name = $name;
        $this->is_item = false;
        $this->units = $units;
        $this->ntoshow = $ntoshow;
        $this->ordered = false;
    }

    // restart this unit
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

function lesson() {
    $name = "";
    $file = "";
    $title = "";
    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'filename': $file = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        } else {
            echo "unprocessed arg of class ".get_class($arg);
        }
    }
    if (!$name) {
        $name = $file;
    }
    if (!$title) {
        $title = $name;
    }
    if (!$file) {
        error_page("Missing filename in lesson");
    }
    return new BoltLesson($name, $title, $file);
}

function exercise() {
    $name = "";
    $file = "";
    $title = "";
    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'filename': $file = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        }
    }
    if (!$name) {
        $name = $file;
    }
    if (!$file) {
        error_page("Missing filename in lesson");
    }
    return new BoltExercise($name, $title, $file);
}

require_once('../inc/bolt_seq.inc');
require_once('../inc/bolt_rnd.inc');
require_once('../inc/bolt_xset.inc');
require_once('../inc/bolt_select.inc');

?>
