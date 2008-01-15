<?php

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/prefs.inc" );
include_once( "../inc/queue.inc" );

$timestr = time_str(time(0));

db_init();

$user        = get_logged_in_user();
$workunitid  = get_int( 'workunitid' );
$workunit    = mysql_fetch_object( mysql_query( "SELECT * FROM workunit WHERE id=".$workunitid ) );
$job         = mysql_fetch_object( mysql_query( "SELECT * FROM q_list WHERE workunit=".$workunitid ) );
$title       = "Deleting job '".workunit_name( $workunit )."' (".$workunitid.") of ".$user -> name." at ".$timestr;

$jobname      = workunit_name( $workunit );

if( $user -> id != $job -> user )
{
 $title  = "Job '".$jobname."' (".$workunitid.")";
 page_head( $title );
 exit_with_text( "You are not the owner of this job !" );
}

$config = get_config();

$jobstatusstring = workunit_status_string( $workunit );
$jobsubmittime   = time_str( $workunit -> create_time );

page_head( $title );
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

$max_jobs = max_nr_of_jobs_of_user( $user );
$njobs = nr_of_jobs_of_user( $user );

row1( "Commands" );
if( $njobs < $max_jobs )
{
 if( $max_jobs - $njobs > 1 )
  $line = "You can submit ".($max_jobs-$njobs)." more jobs: ";
 else
  $line = "You can submit one more job: ";
 row2( $line, '<a href="queue_new_job_form.php">Submit a job</a>' );
}
row2( "", '<a href="queue_show_queue.php">Go back to your queue</a>' );
row2( "", '<a href="logout.php">Log out</a>' );

end_table();

?>
