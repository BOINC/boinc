<?php {

    // $Id$

    // grep logs for a particular string

    require_once("util.inc");

    $log_dir = parse_config("<log_dir>");
    if (!$log_dir) {
        exit("Error: couldn't get log_dir from config file.");
    }

    if (!$f || !preg_match("/^ *([a-z_*]+[.]out *)+$/", $f)) {
        $f = '*.out';
    }

    if ($s) {
        page_head("Grep logs for \"$s\"");
    } else {
        page_head("Show logs");
    }

    echo "<form action=show_log.php>";
    echo " String: <input name=s value='$s'>";
    echo " Files: <input name=f value='$f'>";
    echo " <input type=submit value=Grep></form>";

    passthru("cd $log_dir && ./grep_logs -html '$s' $f 2>&1");

    page_tail();
} ?>
