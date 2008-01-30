<?php

class BoltExerciseSet extends BoltRandom {
    public $repeats;
    function __construct($name, $units, $number, $repeats) {
        parent::__construct($name, $units, $number);
        $this->repeats = $repeats;
    }

    // called when an exercise in this set has just been graded.
    // - record the score in our state structure
    // - return a structure saying what navigation info to show:
    //   - review
    //   - repeat now
    //   - next
    //
    function xset_callback(&$iter, $score, $view_id, &$is_last, &$repeat) {
        global $course;
        global $user;

        echo "XSET_CALLBACK: $score";
        $nav_info = null;
        $state_rec = $iter->state[$this->name];
        $nshown = $state_rec['nshown'];
        $state_rec['scores'][$nshown] = $score;
        $iter->state[$this->name] = $state_rec;
        $is_last = ($nshown+1 == $this->ntoshow);
        if (!$is_last) {
            echo "XSET_CALLBACK: not done";
            return;
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

        echo "XSET_CALLBACK: avg $avg_score";
        $repeat = null;
        $least = 1;
        foreach ($this->repeats as $r) {
            if ($avg_score < $r->score && $r->score<$least) {
                $repeat = $r;
                $least = $r->score;
            }
        }

        $now = time();
        $id = BoltXsetResult::insert("(create_time, user_id, course_id, name, score, view_id) values ($now, $user->id, $course->id, '$this->name', $avg_score, $view_id)");
        $due_time = $now + 100000;
        BoltRefreshRec::replace("create_time=$now, user_id=$user->id, course_id=$course->id, name='$this->name', set_result_id=$id, due_time=$due_time");
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
                echo "start_review: unit $unit_name";
                return true;
            }
        }
        return false;
    }

    function start_repeat(&$iter) {
        $state_rec = $iter->state[$this->name];
        $state_rec['child_name'] = null;
        $state_rec['nshown'] = 0;
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
    $name = "";
    $number = 1;
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'number': $number = $arg[1]; break;
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
        }
    }
    return new BoltExerciseSet($name, $units, $number, $repeats, $refresh);
}

?>
