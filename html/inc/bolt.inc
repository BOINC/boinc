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
        $this->top->walk($this, false, $this->frac_done);
    }

    // move to the next item, and return it in $this->item
    // (null if course finished)
    //
    function next() {
        $this->top->walk($this, true, $this->frac_done);
    }
}

class BoltSequence extends BoltUnit {
    public $units;
    function __construct($n, $u) {
        $this->name = $n;
        $this->units = $u;
        $this->is_item = false;
    }

    function walk(&$iter, $incr, &$frac_done) {
        $n = count($this->units);
        if (array_key_exists($this->name, $iter->state)) {
            $state_rec = $iter->state[$this->name];
            $child_name = $state_rec['child_name'];

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
                    $i++;
                    if ($i == $n) {
                        $frac_done = 1;
                        $state_rec['i'] = 0;
                        $state_rec['child_name'] = null;
                        $iter->state[$this->name] = $state_rec;
                        return true;
                    }
                }
            }
        } else {
            $i = 0;
        }
        $child = $this->units[$i];
        $frac_done = $i/$n;
        $state_rec = null;
        $state_rec['i'] = $i;
        $state_rec['child_name'] = $child->name;
        $iter->state[$this->name] = $state_rec;
        if ($child->is_item) {
            $iter->item = $child;
        } else {
            $child->walk($iter, false, $f);
            $frac_done += $f*(1/$n);
        }
    }
}

// state for random unit is:
// seed         The RNG seed used to shuffle
// i            index of current child
// number_shown number of units shown so far (not necessarily the
//              same as i on 2nd and later pass through this unit)
// child_name   name of current child
class BoltRandom extends BoltUnit {
    public $units;
    function __construct($n, $u, $n) {
        $this->name = $n;
        $this->units = $u;
        $this->is_item = false;
        $this->number = $n;
        $this->shuffled = false;
    }

    function walk(&$iter, $incr, &$frac_done) {
        $n = count($this->units);
        if (array_key_exists($this->name, $iter->state)) {
            $state_rec = $iter->state[$this->name];
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

class BoltItem extends BoltUnit {
    public $filename;
    function __construct($name, $title, $filename) {
        $this->filename = $filename;
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

class BoltRefreshInterval {
    public $intervals;
    function __construct($i) {
        $this->intervals = $i;
    }
}

class BoltReview {
    public $score;
    public $unit;
    function __construct($s, $u) {
        $this->score = $s;
        $this->unit = $u;
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

function refresh_interval($i) {
    return array('refresh_interval', $i);
}

function review($s, $u) {
    return new BoltReview($s, $u);
}

function refresh() {
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

function sequence() {
    $args = func_get_args();
    $units = array();
    $name = "";
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        } else if (is_object($arg)) {
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else {
                echo "Unrecognized arg";
            }
        }
    }
    return new BoltSequence($name, $units);
}

function random() {
    $args = func_get_args();
    $units = array();
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
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else {
                echo "Unrecognized arg";
            }
        }
    }
    return new BoltRandom($name, $units, $number);
}

function exercise_set() {
    $args = func_get_args();
    $units = array();
    $name = "";
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'number': $number = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        } else if (is_object($arg)) {
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else {
                echo "Unrecognized arg";
            }
        }
    }
    return new BoltExerciseSet($name, $units, $number);
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
