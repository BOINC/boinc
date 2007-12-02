<?php

error_reporting(E_ALL);
ini_set('display_startup_errors', true);

// rules about course structures:
//
// - Each unit has a logical name.
// - The members of a set must have distinct logical names
// - Different units may have the same logical name;
//   however, such units should be identical.

class BoltFrame {
    public $state;
        // a data structure that's specific to the unit type,
        // e.g. a loop counter
        // Typically this includes the logical name of the current child.
        // Normally this is implied by the state;
        // however, it may differ if the course structure has changed.
        // In general the unit should restart in this case
    function __construct($state=null) {
        $this->state = $state;
    }
}

abstract class BoltUnit {
    public $name;   // logical name.

    abstract function walk(
        $old_stack, &$new_stack, $next, &$item, &$frac_done
    );
        // multi-purpose function for traversing a course.
        // if $old_stack is null
        //      set up initial state for this unit.
        //      append frames to $new_stack for this unit and descendants
        //      $next is ignored
        //      $item is the initial item
        //      return is ignored
        // else
        //      The first frame of $old_stack is for this unit.
        //      Check for name mismatch (if changed course).
        //      Append frames to $new_stack for this unit and descendants
        //      if $next, the bottom-level non-item unit should increment.
        //      return value: true if the caller should increment
        // frac_done: Fraction done (of this unit and any subunits)
    abstract function is_item();
}

// An iterator represents a user's position in a course.
// It is stored in the database, and the course may change underneath it.
//
class BoltIter {
    public $stack;      // array of BoltFrame
    public $top;

    // point to the start of a course; set up stack.
    //
    function __construct($top) {
        $this->top = $top;
        $this->stack = null;
    }

    // get current item
    //
    function at() {
        $new_stack = array();
        $this->top->walk($this->stack, $new_stack, false, $item, $frac_done);
        $this->stack = $new_stack;
        $x->item = $item;
        $x->frac_done = $frac_done;
        return $x;
    }

    // move to the next item (and return it)
    // return true if we're off the end
    //
    function next() {
        $item = null;
        $new_stack = array();
        $this->top->walk($this->stack, $new_stack, true, $item, $frac_done);
        $this->stack = $new_stack;
        $x->item = $item;
        $x->frac_done = $frac-done;
        return $x;
    }
}

class BoltSeq extends BoltUnit {
    public $units;
    function __construct($n, $u) {
        $this->name = $n;
        $this->units = $u;
    }

    function walk($old_stack, &$new_stack, $next, &$item, &$frac_done) {
        //echo "call to walk() for $this->name: next: $next\n";
        $n = count($this->units);
        if ($old_stack) {
            //echo "old stack: \n";
            //var_dump($old_stack);
            //echo "------------\n";
            $frame = $old_stack[0];
            $state = $frame->state;
            $i = $state->i;

            // first, look up unit by name
            // 
            $child = null;
            for ($j=0; $j<$n; $j++) {
                $c = $this->units[$j];
                if ($c->name == $state->child_name) {
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
                    array_shift($old_stack);
                    $inc = $child->walk(
                        $old_stack, $new_stack, true, $item, $frac_done
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
        $state->i = $i;
        $state->child_name = $child->name;
        $frame = new BoltFrame($state);
        $new_stack[] = $frame;
        if ($child->is_item()) {
            $item = $child;
        } else {
            $child->walk(null, $new_stack, false, $item, $f);
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
    function walk($old_stack, &$new_stack, $next, &$item, &$frac_done) {
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

function Sequence() {
    $args = func_get_args();
    $units = array();
    $name = "";
    foreach ($args as $arg) {
        if (is_object($arg) {
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else if (get_class($arg) == "BoltName")) {
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
