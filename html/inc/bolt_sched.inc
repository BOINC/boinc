<?php

// An iterator represents a user's position in a course.
// Its state is stored in the database,
// and the course may change underneath it.
//
// A state in a course is described by:
// - an associative array mapping logical names to state structures.
//   Typically this includes the logical name of the current child
//   and info such as a sequence index.
//   This is kind of like a "call stack",
//   except that it can contain units not currently active.

//
class BoltIter {
    public $top;        // topmost unit
    public $state;
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

?>
