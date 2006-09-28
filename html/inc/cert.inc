<?

function credit_to_ops($user, &$ops, &$unit) {
    $ops_quadrillion = ($user->total_credit*864)/1000;
    $unit = "quadrillion";
    $ops = $ops_quadrillion;
    if ($ops_quadrillion > 1000) {
        $ops = $ops_quadrillion/1000;
        $unit = "quintillion";
    }
    if ($ops_quadrillion > 1000000) {
        $ops = $ops_quadrillion/1000000;
        $unit = "sextillion";
    }
    if ($ops_quadrillion > 1000000000) {
        $ops = $ops_quadrillion/1000000000;
        $unit = "septillion";
    }
    $ops = number_format($ops, 2);
}

function credit_string($user, $bolden) {
    $cobbs = number_format($user->total_credit, 0);

    credit_to_ops($user, $ops, $unit);
      
    if ($bolden) {
        $lbold="[[";
        $rbold="]]";
    } else {
        $lbold="";        $rbold="";
    } 
    return " $lbold$cobbs Cobblestones$rbold of computation ($ops $unit floating-point operations)";
    return $credit;
}

?>
