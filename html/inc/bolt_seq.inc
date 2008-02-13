<?php

class BoltSequence extends BoltSet {
    function __construct($name, $units) {
        parent::__construct($name, $units, count($units));
    }

    function order() {
        $this->ordered = true;
    }

    function restart(&$iter) {
        $state_rec = $iter->state[$this->name];
        if (!$state_rec) $state_rec = $this->init();
        $state_rec['nshown'] = 0;
        $state_rec['index'] = 0;
        $state_rec['child_name'] = null;
        $iter->state[$this->name] = $state_rec;
    }
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
                echo "Unrecognized arg: ";
                print_r($arg);
            }
        }
    }
    return new BoltSequence($name, $units);
}

?>
