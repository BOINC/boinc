<?php

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/prefs.inc" );
include_once( "../inc/queue.inc" );

db_init();

$user = get_logged_in_user();
$timestr = time_str(time(0));
$title = "Personal job list of ".$user -> name." at ".$timestr;

page_head( $title );

$alljobs = all_jobs_of_user( $user );
$njobs   = mysql_num_rows( $alljobs );

start_table();

if( $njobs ) 
{
 if( $njobs > 1 )
  row1( "You have ".$njobs." jobs listed !<br>" );
 else
  row1( "You have ".$njobs." job listed !<br>" );

 end_table();
 start_table();

 row5( "<b>Job #</b>", "<b>Job submit time</b>", "<b>Job status</b>", "<b>Job name</b>", "<b>Job ID</b>" );

 for( $jobindex = 0; $jobindex < $njobs; ++$jobindex )
 {
  $job = mysql_fetch_object( $alljobs );

  $workunitquery = mysql_query( "SELECT * FROM workunit WHERE id=".$job -> workunit );
  if( $workunitquery )
  {
   $workunit = mysql_fetch_object( $workunitquery );

   if( $workunit )
   {
    $prefix = '<a href="queue_show_job.php?workunitid='.$job -> workunit.'">';
    $workunitname = $prefix.workunit_name( $workunit ).'</a>';
    $status = workunit_status_string( $workunit );
    if( $status == "running" ) $status = "<font color='green'><b>".$status."</b></font>";
    if( $status == "queued" ) $status = "<font color='blue'><b>".$status."</b></font>";
    if( $status == "ERROR" ) $status = "<font color='red'><b>".$status."</b></font>";
    $jobsubmittime = time_str( $workunit -> create_time );
   }
   else
   {
    $workunitname = "<font color='red'>WORKUNIT NOT FOUND IN DATABASE</font>";
    $status = "<font color='red'>UNKNOWN</font>";
    $jobsubmittime = "<font color='red'>UNKNOWN</font>";
   }

   mysql_free_result( $workunitquery );
  }

  $workunitstring = "<a href=workunit.php?wuid=".$job -> workunit.">".$job -> workunit."</a>";
  row5( $jobindex+1, $jobsubmittime, $status, $workunitname, $workunitstring );
 }

}
else
 row1("You have NO jobs listed !<br>");

end_table();

$max_jobs = max_nr_of_jobs_of_user( $user );

if( $max_jobs > $njobs )
{
 if( $max_jobs - $njobs > 1 )
  $line = 'You can submit '.( $max_jobs - $njobs ).' more jobs: ';
 else
  $line = 'You can submit one more job: ';

 start_table();
 row1( "Commands" );
 row2( $line, '<a href="queue_new_job_form.php">Submit a job</a>' );
 row2( "", '<a href="home.php">Your account</a>' );
 row2( "", '<a href="hosts_user.php">Your computers</a>' );
 row2( "", '<a href="logout.php">Log out</a>' );
 end_table();
}
else
 exit_with_text( "You cannot submit any more jobs, you have reached your limit, clean up first !" );

page_tail(); 
?>
