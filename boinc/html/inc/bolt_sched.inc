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

// An iterator represents a user's position in a course.
// Its state is stored in the database,
// and the course may change underneath it.
//
// A state in a course is described by an associative array
// mapping logical names to state structures.
// Typically this includes the logical name of the current child
// and info such as a sequence index.
// This is kind of like a "call stack",
// except that it can contain units not currently active.

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
        $this->state = unserialize($encoded_state);
    }

    function encode_state() {
        return serialize($this->state);
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
