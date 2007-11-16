<?php

// limit a given host to 1 job per day

// TODO: document; use new DB interface

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );

db_init();

if (get_int('hostid')) {
    $hostid = get_int( 'hostid' );
} else {
    error_page("no hostid");
}

$timestr = time_str(time(0));
$title = "host ".$hostid." max_results_day set to 1 at ".$timestr;

admin_page_head( $title );

if($hostid > 0) {
    $result = mysql_query("UPDATE host SET max_results_day=1 WHERE id=".$hostid);
}

echo $title;

admin_page_tail();

?>
