<?php

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

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
        //echo "call to walk() for $this->name: next: $next\n";
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
        $state_rec->i = $i;
        $state_rec->child_name = $child->name;
        $iter->state[$this->name] = $state_rec;
        if ($child->is_item) {
            $iter->item = $child;
        } else {
            $child->walk($iter, false, $f);
            $frac_done += $f*(1/$n);
        }
    }
}

class BoltItem extends BoltUnit {
    public $filename;
    function __construct($name, $filename) {
        $this->filename = $filename;
        $this->name = $name;
        $this->is_item = true;
    }
    function begin() {
        return array(new BoltFrame($this));
    }
    function unit_list() {
        return array(&$this);
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

class BoltName {
    public $name;
    function __construct($n) {
        $this->name = $n;
    }
}

class BoltFileName {
    public $fname;
    function __construct($n) {
        $this->fname = $n;
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
    return new BoltName($n);
}

function filename($n) {
    return new BoltFileName($n);
}

function refresh_interval($i) {
    return new BoltRefreshInterval($i);
}

function review($s, $u) {
    return new BoltReview($s, $u);
}

function lesson() {
    $name = "";
    $file = "";
    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_object($arg)) {
            if (get_class($arg) == 'BoltName') {
                $name = $arg->name;
            } else if (get_class($arg) == 'BoltFileName') {
                $fname = $arg->fname;
            } else {
                echo "unprocessed arg of class ".get_class($arg);
            }
        }
    }
    if (!$name) {
        error_page("Missing name in lesson");
    }
    if (!$fname) {
        error_page("Missing filename in lesson");
    }
    return new BoltLesson($name, $fname);
}

function exercise() {
    $name = "";
    $file = "";
    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_object($arg)) {
            if (get_class($arg) == 'BoltName') {
                $name = $arg->name;
            } else if (get_class($arg) == 'BoltFileName') {
                $fname = $arg->fname;
            }
        }
    }
    if (!$name || !$fname) {
        error_page("Missing name or filename in exercise");
    }
    return new BoltExercise($name, $fname);
}

function sequence() {
    $args = func_get_args();
    $units = array();
    $name = "";
    foreach ($args as $arg) {
        if (is_object($arg)) {
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else if (get_class($arg) == "BoltName") {
                $name = $arg->name;
            } else {
                echo "Unrecognized arg";
            }
        }
    }
    return new BoltSequence($name, $units);
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

function info_incomplete($user) {
    if (!$user->bolt->birth_year) return true;
    if (!$user->bolt->sex) return true;
    return false;
}

function birth_year_select($user) {
    $this_year = date("Y");
    $x = "<select name=birth_year>\n";
    for ($i=$this_year-100; $i<$this_year; $i++) {
        $s = ($i == $user->bolt->birth_year)?"selected":"";
        $x .= "<option value=$i $s>$i</option>\n";
    }
    $s = (!$user->bolt->birth_year)?"selected":"";
        $x .= "<option value=$0 $s>Unspecified</option>\n";
    $x .= "</select>\n";
    return $x;
}

function sex_select($user) {
    $x = "<select name=sex>\n";
    $s = ($user->bolt->sex == 0)?"selected":"";
    $x .= "<option value=0 $s>Unspecified</option>\n";
    $s = ($user->bolt->sex == 1)?"selected":"";
    $x .= "<option value=1 $s>Male</option>\n";
    $s = ($user->bolt->sex == 2)?"selected":"";
    $x .= "<option value=2 $s>Female</option>\n";
    $x .= "</select>\n";
    return $x;
}

function request_info($user, $course) {
    page_head("Student info");
    echo "<form action=bolt_sched.php>
        <input type=hidden name=action value=update_info>
        <input type=hidden name=course_id value=$course->id>
    ";
    start_table();
    row2("Birth year", birth_year_select($user));
    row2("Sex", sex_select($user));
    row2("", "<input type=submit value=OK>");
    end_table();
    echo "</form>\n";
    page_tail();
}

?>
