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

class BoltExerciseSet extends BoltRandom {
    public $repeats;
    public $refresh;
    public $weight;
    public $callback;

    function __construct(
        $name, $units, $number, $repeats, $refresh, $attrs, $callback, $weight
    ) {
        parent::__construct($name, $units, $number, $attrs);

        $this->repeats = $repeats;
        $this->refresh = $refresh;
        $this->callback = $callback;
        $this->weight = $weight;
    }

    // called when an exercise in this set has just been graded.
    // - record the score in our state structure
    // - return true if this was last exercise in the set
    // - if so, also return a structure saying what navigation info to show:
    //   - review
    //   - repeat now
    //   - next
    //
    function xset_record_score(
        &$iter, $score, $view_id, &$avg_score, &$repeat
    ) {
        global $course;
        global $user;

        $nav_info = null;
        $state_rec = $iter->state[$this->name];
        $nshown = $state_rec['nshown'];
        $state_rec['scores'][$nshown] = $score;
        $iter->state[$this->name] = $state_rec;
        $is_last = ($nshown+1 == $this->ntoshow);
        if (!$is_last) {
            return false;
        }

        // this exercise set is now "completed".
        //   - create exercise_set_result record
        //   - optionally create or update bolt_refresh record
        //
        $total_score = 0;
        for ($i=0; $i<$nshown+1; $i++) {
            $total_score += $state_rec['scores'][$i];
        }
        $avg_score = $total_score/($nshown+1);

        $repeat = null;
        $least = 2;
        foreach ($this->repeats as $r) {
            if ($avg_score < $r->score && $r->score<$least) {
                $repeat = $r;
                $least = $r->score;
            }
        }
        return true;
    }

    function walk(&$iter, $incr, &$frac_done) {
        $iter->xset = $this;
        // see if we're doing a review
        //
        if (array_key_exists($this->name, $iter->state)) {
            $state_rec = $iter->state[$this->name];
            $child_name = $state_rec['child_name'];
            foreach ($this->repeats as $r) {
                if ($r->unit && ($r->unit->name == $child_name)) {
                    // we're doing a review
                    //
                    $child = $r->unit;
                    if ($incr) {
                        if ($child->is_item) {
                            $my_inc = true;
                        } else {
                            $my_inc = $child->walk($iter, $incr, $frac_done);
                        }
                        if ($my_inc) {
                            // we're done with review.  do exercises again
                            //
                            $state_rec['child_name'] = null;
                            $state_rec['nshown'] = 0;
                            $iter->state[$this->name] = $state_rec;
                        }
                    } else {
                        if ($child->is_item) {
                            $iter->item = $child;
                        } else {
                            $child->walk($iter, false, $f);
                        }
                    }
                    return false;
                }
            }
        }

        return parent::walk($iter, $incr, $frac_done);
    }

    function start_review(&$iter, $unit_name) {
        foreach ($this->repeats as $r) {
            if ($r->unit->name == $unit_name) {
                $state_rec = $iter->state[$this->name];
                $state_rec['child_name'] = $unit_name;
                $iter->state[$this->name] = $state_rec;
                if (!$r->unit->is_item) {
                    $r->unit->restart($iter);
                }
                return true;
            }
        }
        return false;
    }
}

class BoltRefresh{
    public $intervals;
    function __construct($i) {
        $this->intervals = $i;
    }
}

class BoltRepeat {
    public $score;
    public $unit;
    public $flags;
    function __construct($s, $u, $f) {
        $this->score = $s;
        $this->unit = $u;
        $this->flags = $f;
    }
}

define('REVIEW', 1);
define('REPEAT', 2);
define('NEXT', 4);

function repeat($s, $u, $f) {
    return new BoltRepeat($s, $u, $f);
}

function refresh($a) {
    return new BoltRefresh($a);
}

function exercise_set() {
    $args = func_get_args();
    $units = array();
    $repeats = array();
    $refresh = null;
    $callback = null;
    $name = "";
    $number = 0;
	$attrs = null;
    $weight = 1;
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'number': $number = $arg[1]; break;
            case 'attrs': $attrs = $arg[1]; break;
            case 'callback': $callback = $arg[1]; break;
            case 'weight': $weight = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        } else if (is_object($arg)) {
            if (get_class($arg) == "BoltExercise") {
                $units[] = $arg;
            } else if (get_class($arg) == "BoltRepeat") {
                $repeats[] = $arg;
            } else if (get_class($arg) == "BoltRefresh") {
                $refresh= $arg;
            } else {
                echo "Can't include object of type ".get_class($arg)." within an exercise set.";
            }
        } else {
            echo "Unexpected arg to exercise_set(): "; print_r($arg);
        }
    }

    if ($number == 0) $number = count($units);
    return new BoltExerciseSet(
        $name, $units, $number, $repeats, $refresh, $attrs, $callback, $weight
    );
}

?>
