<?php {

    // $Id$

    // grep logs for a particular string

    require_once("util.inc");

    $log_dir = parse_config("<log_dir>");
    if (!$log_dir) {
        exit("Error: couldn't get log_dir from config file.");
    }

    $f = $_GET["f"];
    $s = $_GET["s"];

    if (!$f || !preg_match("/^ *([a-z_*]+[.](log|out) *)+$/", $f)) {
        $f = '*.log';
    }

    if ($s) {
        page_head("Grep logs for \"$s\"");
    } else {
        page_head("Show logs");
    }

    echo "<form action=show_log.php>";
    echo " Regexp: <input name=s value='$s'>";
    echo " Files: <input name=f value='$f'>";
    echo " <input type=submit value=Grep></form>";

    echo 'Hint: Example greps: "RESULT#106876", "26fe99aa_25636_00119.wu_1", "WU#8152", "too many errors", "2003-07-17", "CRITICAL" <br>';

    passthru("cd $log_dir && ../bin/grep_logs -html '$s' $f 2>&1");

    page_tail();
} ?>
