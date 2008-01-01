<?php

// state for random unit is:
// seed         The RNG seed used to shuffle
// i            index of current child
// number_shown number of units shown so far (not necessarily the
//              same as i on 2nd and later pass through this unit)
// child_name   name of current child
class BoltRandom extends BoltUnit {
    public $units;
    function __construct($name, $units, $number) {
        $this->name = $name;
        $this->units = $units;
        $this->number = $number;
        $this->is_item = false;
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
