<?php

class BoltSequence extends BoltUnit {
    public $units;
    function __construct($name, $units) {
        $this->name = $name;
        $this->units = $units;
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
