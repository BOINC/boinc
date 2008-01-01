<?php

class BoltExerciseSet extends BoltUnit {
    public $units;
    public $repeats;
    public $state_rec;
    function __construct($name, $units, $number, $repeats) {
        $this->name = $name;
        $this->units = $units;
        $this->number = $number;
        $this->repeats = $repeats;
        $this->is_item = false;
        $this->shuffled = false;
    }

    // the scheduler calls this when an exercise in this set
    // has just been graded.
    // - record the score
    // - if this is the last exercise in the set,
    //   create exercise_set_result record
    //   and optionally create or update bolt_refresh record
    // - return a structure saying what navigation info to show:
    //   - review
    //   - repeat now
    //   - next
    //
    function xset_callback($iter, $score, &$is_last, &$nav_info) {
        $state_rec = $iter->state[$this->name];
        $number_shown = $state_rec['number_shown'];
        $state_rec['scores'][$number_shown] = $score;
        if ($number_shown == $this->number) {
            $is_last = false;
            return;
        }
        $is_last = true;
        BoincResult::insert("");
        BoincRefresh::replace("");
    }

    function walk(&$iter, $incr, &$frac_done) {
        $iter->xset = $this;
        $n = count($this->units);
        if (array_key_exists($this->name, $iter->state)) {
            $state_rec = $iter->state[$this->name];
            $this->state_rec = $state_rec;
            $child_name = $state_rec['child_name'];
            $number_shown = $state_rec['number_shown'];
            if (!$this->shuffled) {
                srand($state_rec['seed']);
                shuffle($this->units);
                $this->shuffled = true;
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
                $i = $state_rec['i'];
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
                $my_inc = false;
                if ($child->is_item) {
                    $my_inc = true;
                } else {
                    $my_inc = $child->walk($iter, $incr, $frac_done);
                }
                if ($my_inc) {
                    $i = ($i+1)%$n;
                    $number_shown++;
                    if ($number_shown >= $this->number) {
                        $frac_done = 1;
                        $state_rec['i'] = $i;
                        $state_rec['number_shown'] = 0;
                        $state_rec['child_name'] = null;
                        $iter->state[$this->name] = $state_rec;
                        return true;
                    }
                }
            }
        } else {
            $i = 0;
            $number_shown = 0;
            $state_rec = null;
            $seed = ((double)microtime()*1000000);
            srand($seed);
            shuffle($this->units);
            $state_rec['seed'] = $seed;
        }
        $child = $this->units[$i];
        $frac_done = $number_shown/$this->number;
        $state_rec['i'] = $i;
        $state_rec['number_shown'] = $number_shown;
        $state_rec['child_name'] = $child->name;
        $iter->state[$this->name] = $state_rec;
        if ($child->is_item) {
            $iter->item = $child;
        } else {
            $child->walk($iter, false, $f);
            $frac_done += $f*(1/$number);
        }
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
                echo "Unrecognized object arg: ";
                print_r($arg);
            }
        }
    }
    return new BoltExerciseSet($name, $units, $number, $repeats, $refresh);
}

?>
