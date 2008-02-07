<?php

class BoltRandom extends BoltSet {
    public $units;
    function __construct($name, $units, $number) {
        parent::__construct($name, $units, $number);
    }

    function order(&$iter) {
        $state_rec = $iter->state[$this->name];
        if ($state_rec) {
            if (array_key_exists('seed', $state_rec)) {
                $seed = $state_rec['seed'];
            } else {
                $seed = ((double)microtime()*1000000);
                $state_rec['seed'] = $seed;
                $iter->state[$this->name] = $state_rec;
            }
        } else {
            $state_rec = $this->init();
            $seed = ((double)microtime()*1000000);
            $state_rec['seed'] = $seed;
            $iter->state[$this->name] = $state_rec;
        }
        srand($seed);
        shuffle($this->units);
        $this->ordered = true;
    }
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
                echo "Unrecognized arg: ";
                print_r($arg);
            }
        }
    }
    return new BoltRandom($name, $units, $number);
}

?>
