<?php

function select_cmp($a, $b) {
    if ($a->value == $b->value) return 0;
    return ($a->value < $b->value)?1:-1;
}

class BoltSelect extends BoltSet {
    public $valuator;

    function __construct($name, $units, $valuator) {
        $this->valuator = $valuator;
        parent::__construct($name, $units, 1);
    }

    function order() {
        global $student;
        foreach ($this->units as $u) {
            $func = $this->valuator;
            $u->value = $func($student, $u);
        }
        usort($this->units, 'select_cmp');
        $this->ordered = true;
    }

    function finished() {
        global $user;
        global $course;
        global $view;

        echo "FINISHED: $child_name";
        $now = time();
        BoltSelectFinished::insert("(user_id, course_id, end_time, name, view_id) values ($user->id, $course->id, $now, '$this->name', $view->id)");
    }
}

function valuator($n) {
    return array('valuator', $n);
}

function select() {
    $args = func_get_args();
    $units = array();
    $name = "";
    foreach ($args as $arg) {
        if (is_array($arg)) {
            switch ($arg[0]) {
            case 'name': $name = $arg[1]; break;
            case 'title': $title = $arg[1]; break;
            case 'valuator': $valuator = $arg[1]; break;
            default: echo "Unrecognized array arg: ", $arg[0], "\n"; break;
            }
        } else if (is_object($arg)) {
            if (is_subclass_of($arg, "BoltUnit")) {
                $units[] = $arg;
            } else {
                echo "Unrecognized arg: ";
                print_r($arg);
            }
        }
    }
    if (!$valuator) {
        error_page("missing valuator");
    }
    return new BoltSelect($name, $units, $valuator);
}

?>
