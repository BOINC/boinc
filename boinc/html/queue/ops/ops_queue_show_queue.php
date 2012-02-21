<?php

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );
include_once( "../inc/queue.inc" );

db_init();

$timestr = time_str(time(0));
$title = "Job list at ".$timestr;

admin_page_head( $title );

$alljobs = mysql_query( "SELECT * FROM workunit WHERE name LIKE '%queue%'" );
$njobs   = mysql_num_rows( $alljobs );

start_table();

if( $njobs ) 
{
 if( $njobs > 1 )
  row1( "There are ".$njobs." jobs listed !<br>" );
 else
  row1( "There is only one job listed !<br>" );

 end_table();
 start_table();

 row6( "<b>Job #</b>", "<b>User</b>", "<b>Job submit time</b>", "<b>Job status</b>", "<b>Job name</b>", "<b>Job ID</b>" );

 for( $jobindex = 0; $jobindex < $njobs; ++$jobindex )
 {
  $workunit = mysql_fetch_object( $alljobs );

  $prefix = '<a href="queue_show_job.php?workunitid='.$workunit -> id.'">';
  $workunitname = $prefix.workunit_name( $workunit ).'</a>';
  $workunitidstr = "<a href='db_action.php?table=workunit&id=".$workunit -> id."'>".$workunit -> id."</a>";
  $status = workunit_status_string( $workunit );
  $jobsubmittime = time_str( $workunit -> create_time );

  if( $status != "CANCELED" )
  {
   if( $status == "running" ) $status = "<font color='green'><b>".$status."</b></font>";
   if( $status == "queued" ) $status = "<font color='blue'><b>".$status."</b></font>";

   $job = mysql_fetch_object( mysql_query( "SELECT * FROM q_list WHERE workunit=".$workunit->id ) );
   $user = mysql_fetch_object( mysql_query( "SELECT * FROM user WHERE id=".$job -> user ) );
   $jobusername = "<a href='db_action.php?table=user&id=".$user -> id."'>".$user -> name."</a>";
  }
  else
  {
   $jobusername = "<font color='red'><b>UNKNOWN</b></font>";
   $status = "<font color='red'><b>CANCELED</b></font>";
   $workunitname = workunit_name( $workunit );
  }

  row6( $jobindex+1, $jobusername, $jobsubmittime, $status, $workunitname, $workunitidstr );
 }

}
else
 row1("There are NO jobs listed !<br>");

end_table();

admin_page_tail();

?>
