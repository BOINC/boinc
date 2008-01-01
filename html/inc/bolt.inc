<?php

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

// stuff needed in Bolt course documents

// rules about course structures:
//
// - Each unit has a logical name.
// - The members of a set must have distinct logical names
// - Different units may have the same logical name;
//   however, such units must be identical.

// A state in a course is described by an associative array
// mapping logical names to state structures.
// Typically this includes the logical name of the current child
// and info such as a sequence index.

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

// An iterator represents a user's position in a course.
// Its state is stored in the database,
// and the course may change underneath it.
//
class BoltIter {
    public $top;        // topmost unit
    public $state;      // state array
    public $xset;       // exercise set, if any

    // the following are temps
    public $item;       // current item
    public $frac_done;  // fraction done

    function __construct($top) {
        $this->top = $top;
        $this->state = array();
    }

    function decode_state($encoded_state) {
        $this->state = json_decode($encoded_state, true);
    }

    function encode_state() {
        return json_encode($this->state);
    }

    // get current item and fraction done
    //
    function at() {
        $this->xset = null;
        $this->top->walk($this, false, $this->frac_done);
    }

    // move to the next item, and return it in $this->item
    // (null if course finished)
    //
    function next() {
        $this->top->walk($this, true, $this->frac_done);
    }
}

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
        error_page("Missing name in lesson");
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
        error_page("Missing name in lesson");
    }
    if (!$title) {
        $title = $name;
    }
    if (!$file) {
        error_page("Missing filename in lesson");
    }
    return new BoltExercise($name, $title, $file);
}

function enum_course($course) {
    $iter = new BoltIter($course);
    while (1) {
        $x = $iter->at();
        if (!$x) break;
        echo "at: $x->url\n";
        $x = $iter->next();
        if (!$x) break;
        echo "next: $x->filename\n";
    }
    echo "course over\n";
}

require_once('../inc/bolt_seq.inc');
require_once('../inc/bolt_rnd.inc');
require_once('../inc/bolt_xset.inc');

?>
