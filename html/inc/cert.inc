<?

function credit_to_ops($user, &$ops, &$unit) {
    // 100 units of credit is 86400*10^9 ops
    // 1 unit of credit is 864*10^9 ops
    $ops_trillion = ($user->total_credit*864)/1000;
    $unit = "trillion";
    $ops = $ops_trillion;
    if ($ops_trillion > 1000) {
        $ops = $ops_trillion/1000;
        $unit = "quadrillion";
    }
    if ($ops_trillion > 1000000) {
        $ops = $ops_trillion/1000000;
        $unit = "quintillion";
    }
    if ($ops_trillion > 1000000000) {
        $ops = $ops_trillion/1000000000;
        $unit = "sextillion";
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
