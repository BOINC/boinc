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

// A position in a course (i.e. the last item viewed) is described by
// 1) a "path" - a list of names.
// 2) an associative array mapping name to state structures.
//  e.g. a loop counter
//  Typically this includes the logical name of the current child.
//  Normally this is implied by the state;
//  however, it may differ if the course structure has changed.
//  In general the unit should restart in this case

abstract class BoltUnit {
    public $name;   // logical name.

    abstract function walk(
        &$state, $old_path, &$new_path, $next, &$item, &$frac_done
    );
        // multi-purpose function for traversing a course.
        // if $old_path is null
        //      This unit is being started;
        //      append names to $new_path for this unit and descendants
        //      $next is ignored
        //      $item is the initial item
        //      return is ignored
        // else
        //      This unis is being resumed in the middle.
        //      The first entry in $old_path is for this unit.
        //      Check for name mismatch (if changed course).
        //      Append names to $new_path for this unit and descendants
        //      if $next, the bottom-level non-item unit should increment.
        //      return value: true if the caller should increment
        // frac_done: Fraction done (of this unit and any subunits)
    abstract function is_item();
}

// An iterator represents a user's position in a course.
// Its state is stored in the database,
// and the course may change underneath it.
//
class BoltIter {
    public $path;       // array of names
    public $top;        // topmost unit
    public $state;      // state array
    public $item;       // current item
    public $frac_done;  // fraction done

    // point to the start of a course; set up path.
    //
    function __construct($top) {
        $this->top = $top;
        $this->path = null;
        $this->state = array();
    }

    function decode_state($encoded_state) {
        $this->state = json_decode($encoded_state, true);
        print_r($this->state);
        $this->path = $this->state['path'];
    }

    function encode_state() {
        $this->state['path'] = $this->path;
        print_r($this->state);
        return json_encode($this->state);
    }

    // get current item
    //
    function at() {
        $new_path = array();
        $this->top->walk($this->state, $this->path, $new_path, false, $item, $frac_done);
        $this->path = $new_path;
        $this->item = $item;
        $this->frac_done = $frac_done;
    }

    // move to the next item (and return it)
    // return true if we're off the end
    //
    function next() {
        $item = null;
        $new_path = array();
        $this->top->walk($this->state, $this->path, $new_path, true, $item, $frac_done);
        $this->path = $new_path;
        $this->item = $item;
        $this->frac_done = $frac_done;
    }
}

class BoltSequence extends BoltUnit {
    public $units;
    function __construct($n, $u) {
        $this->name = $n;
        $this->units = $u;
    }

    function walk(&$state, $old_path, &$new_path, $next, &$item, &$frac_done) {
        //echo "call to walk() for $this->name: next: $next\n";
        $n = count($this->units);
        if ($old_path) {
            //echo "old path: \n";
            //var_dump($old_path);
            //echo "------------\n";
            $child_name = $old_path[0];
            $state_rec = $state[$this->name];
            $i = $state_rec['i'];

            // first, look up unit by name
            // 
            $child = null;
            for ($j=0; $j<$n; $j++) {
                $c = $this->units[$j];
                if ($c->name == $child_name) {
                    $child = $c;
                    break;
                }
            }

            // if not there, look up by index
            //
            if (!$child) {
                if ($i >= $n) {
                    // and if index is too big, use last unit
                    //
                    $i = $n-1;
                }
                $child = $this->units[$i];
            }

            // at this point, $child is the right unit
            //
            if ($next) {
                $inc = false;
                if ($child->is_item()) {
                    $inc = true;
                } else {
                    array_shift($old_path);
                    $inc = $child->walk(
                        $state, $old_path, $new_path, true, $item, $frac_done
                    );
                }
                if ($inc) {
                    $i++;
                    if ($i == $n) {
                        $frac_done = 1;
                        return true;
                    }
                }
            }
        } else {
            $i = 0;
        }
        $frac_done = $i/$n;
        $child = $this->units[$i];
        $state_rec = null;
        $state_rec->i = $i;
        $state_rec->child_name = $child->name;
        $new_path[] = $child->name;
        $state[$this->name] = $state_rec;
        if ($child->is_item()) {
            $item = $child;
        } else {
            $child->walk($state, null, $new_stack, false, $item, $f);
            $frac_done += $f*(1/$n);
        }
    }
    function is_item() {
        return false;
    }
}

class BoltItem extends BoltUnit {
    public $filename;
    function __construct($name, $filename) {
        $this->filename = $filename;
        $this->name = $name;
    }
    function begin() {
        return array(new BoltFrame($this));
    }
    function unit_list() {
        return array(&$this);
    }
    function is_item() {
        return true;
    }
    function walk(&$state, $old_stack, &$new_stack, $next, &$item, &$frac_done) {
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
