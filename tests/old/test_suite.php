#! /usr/local/bin/php
<?php
    //Run the comprehensive suite of tests
    //

    passthru("./test_1sec.php", $retval);
    if($retval) printf("./test_1sec.php did not run correctly\n");
    sleep(3);

    PassThru("./test_backend.php", $retval);
    if($retval) printf("./test_backend.php did not run correctly\n");
    sleep(3);

    PassThru("./test_concat.php", $retval);
    if($retval) printf("./test_concat.php did not run correctly\n");
    sleep(3);

    PassThru("./test_rsc.php", $retval);
    if($retval) printf("./test_rsc.php did not run correctly\n");
    sleep(3);

    PassThru("./test_sticky.php", $retval);
    if($retval) printf("./test_sticky.php did not run correctly\n");
    sleep(3);

    passthru("./test_uc.php", $retval);
    if($retval) printf("./test_uc.php did not run correctly\n");
?>
