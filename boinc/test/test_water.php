#! /usr/local/bin/php
<?php
    //make sure that the scheduler sends the correct number of wu's in test_uc

    PassThru("test_uc.php | grep -c '<scheduler_request>' > tw_results", 
        $retval
    );
    if($retval) {
        echo "test_uc.php did not run correctly\n";
    }
    $f = fopen("tw_results", "r");
    fscanf($f, "%d", $val);
    if($val > 2) {
        echo "Water marks are working\n";
    } else {
        echo "Water marks are not working\n";
    }
    PassThru("rm -f tw_results");
?>
