<?php

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );
include_once( "../inc/queue.inc" );

$timestr = time_str(time(0));

db_init();

$workunitid  = get_int( 'workunitid' );

$workunit    = mysql_fetch_object( mysql_query( "SELECT * FROM workunit WHERE id=".$workunitid ) );
$job         = mysql_fetch_object( mysql_query( "SELECT * FROM q_list WHERE workunit=".$workunit->id ) );
$user        = mysql_fetch_object( mysql_query( "SELECT * FROM user WHERE id=".$job -> user ) );
$title       = "Deleting job '".workunit_name( $workunit )."' (".$workunitid.") of ".$user -> name." at ".$timestr;

$jobname      = workunit_name( $workunit );

$config = get_config();

$jobstatusstring = workunit_status_string( $workunit );
$jobsubmittime   = time_str( $workunit -> create_time );

admin_page_head( $title );
start_table();
row1( "Job speciffics" );
row2( "Job submit time: ", $jobsubmittime );
row2( "Job name: ", $jobname );
row2( "Old job status: ", $jobstatusstring );

$query = "UPDATE result SET server_state=5,outcome=5 WHERE server_state=2 AND workunitid=".$workunit -> id;
mysql_query( $query );

$query = "UPDATE workunit SET error_mask=error_mask|16,transition_time=".time(0).",batch=0 WHERE id=".$workunit -> id;
mysql_query( $query );

$query = "DELETE FROM q_list WHERE id=".$job -> id;
mysql_query( $query );

row2( "New job status: ", "deleted" );

row1( "Commands" );
row2( "", '<a href="queue_show_queue.php">Go back to queue</a>' );

end_table();
admin_page_tail();

?>
