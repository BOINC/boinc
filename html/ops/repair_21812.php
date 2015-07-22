<?php

// fix DB damage caused by a bug introduced in [21181].
// If you ran a server using code between this and [21812], run this script.

$cli_only = true;
require_once("../inc/db.inc");

set_time_limit(0);
db_init_aux();

$config = get_config();
$mjd = parse_element($config, "daily_result_quota");
if (!$mjd) {
    $mjd = 100;
}

function do_query($query) {
    echo "Doing query:\n$query\n";
    $result = _mysql_query($query);
    if (!$result) {
        echo "Failed:\n"._mysql_error()."\n";
    } else {
        echo "Success.\n";
    }
}
do_query("update host_app_version set turnaround_var=0, turnaround_q=0");
do_query("update host_app_version set max_jobs_per_day=$mjd where max_jobs_per_day>$mjd or max_jobs_per_day<0");
do_query("update host_app_version set consecutive_valid=0 where consecutive_valid<0 or consecutive_valid>1000");

?>
