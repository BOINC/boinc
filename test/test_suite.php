#! /usr/local/bin/php
<?php
    //Run the comprehensive suite of tests
    //

    PassThru("test_1sec.php", $retval);
    if($retval) printf("test_1sec.php did not run correctly\n");

    PassThru("test_concat.php", $retval);
    if($retval) printf("test_concat.php did not run correctly\n");

    PassThru("test_rsc.php", $retval);
    if($retval) printf("test_rsc.php did not run correctly\n");

    PassThru("test_uc.php", $retval);
    if($retval) printf("test_uc.php did not run correctly\n");
?>
