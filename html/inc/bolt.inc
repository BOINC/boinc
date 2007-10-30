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

    abstract function walk($old_stack, &$new_stack, $next, &$item);
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
        $this->top->walk($this->stack, $new_stack, false, $item);
        $this->stack = $new_stack;
        return $item;
    }

    // move to the next item (and return it)
    // return true if we're off the end
    //
    function next() {
        $item = null;
        $new_stack = array();
        $this->top->walk($this->stack, $new_stack, true, $item);
        $this->stack = $new_stack;
        return $item;
    }
}

class BoltSeq extends BoltUnit {
    public $units;
    function __construct($n, $u) {
        $this->name = $n;
        $this->units = $u;
    }

    function walk($old_stack, &$new_stack, $next, &$item) {
        //echo "call to walk() for $this->name: next: $next\n";
        if ($old_stack) {
            //echo "old stack: \n";
            //var_dump($old_stack);
            //echo "------------\n";
            $frame = $old_stack[0];
            $state = $frame->state;
            $i = $state->i;
            $restart = false;
            if ($i >= count($this->units)) {
                echo "index too large - restarting\n";
                $restart = true;
            }
            $child = $this->units[$i];
            if ($state->child_name != $child->name) {
                echo "bad name - restarting\n";
                $restart = true;
            }
            if (!$restart) {
                if ($next) {
                    $inc = false;
                    if ($child->is_item()) {
                        $inc = true;
                    } else {
                        array_shift($old_stack);
                        $inc = $child->walk($old_stack, $new_stack, true, &$item);
                    }
                    if ($inc) {
                        $i++;
                        if ($i == count($this->units)) {
                            return true;
                        }
                    }
                }
            }
        } else {
            $restart = true;
        }
        if ($restart) {
            $i = 0;
        }
        $child = $this->units[$i];
        $state->i = $i;
        $state->child_name = $child->name;
        $frame = new BoltFrame($state);
        $new_stack[] = $frame;
        if ($child->is_item()) {
            $item = $child;
        } else {
            $child->walk(null, $new_stack, false, $item);
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
    function walk($old_stack, &$new_stack, $next, &$item) {
        echo "SHOULDN'T BE HERE\n";
    }
}

class BoltLesson extends BoltItem {
}

class BoltExercise extends BoltItem {
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
