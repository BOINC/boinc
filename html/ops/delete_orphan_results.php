<?php

// delete results without a corresponding workunit.
// (in principle these shouldn't exist)

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/boinc_db.inc");

$ndel = 0;
while (1) {
    $rs = BoincResult::enum("true order by id limit 100");
    $found = false;
    foreach ($rs as $r) {
        $wu = BoincWorkunit::lookup_id($r->workunitid);
        if ($wu) {
            echo "$r->id has a WU\n";
            $found = true;
            break;
        } else {
            echo "$r->id has no WU - deleting\n";
            $ndel++;
            $r->delete();
        }
    }
    if ($found) break;
}
echo "Done - deleted $ndel results\n";

?>
