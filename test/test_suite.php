#! /usr/local/bin/php
<?php
    //Run the comprehensive suite of tests
    //

    PassThru("test_api.php", $retval);
    if($retval) printf("test_api.php did not run correctly\n");

    //PassThru("test_dynamic.php", $retval);
    //if($retval) printf("test_dynamic.php did not run correctly\n");

    //PassThru("test_projects.php", $retval);
    //if($retval) printf("test_projects.php did not run correctly\n");

    PassThru("test_stderr.php", $retval);
    if($retval) printf("test_stderr.php did not run correctly\n");

    PassThru("test_uc.php", $retval);
    if($retval) printf("test_uc.php did not run correctly\n");

    PassThru("test_water.php", $retval);
    if($retval) printf("test_water.php did not run correctly\n");

    //PassThru("test_1sec.php", $retval);
    //if($retval) printf("test_1sec.php did not run correctly\n");

    PassThru("test_concat.php", $retval);
    if($retval) printf("test_concat.php did not run correctly\n");

    //PassThru("test_prefs.php", $retval);
    //if($retval) printf("test_prefs.php did not run correctly\n");

    PassThru("test_rsc.php", $retval);
    if($retval) printf("test_rsc.php did not run correctly\n");

    //PassThru("test_uc_slow.php", $retval);
    //if($retval) printf("test_uc_slow.php did not run correclty\n");
?>
