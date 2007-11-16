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
$title       = "Job '".workunit_name( $workunit )."' (".$workunitid.") of ".$user -> name." at ".$timestr;

$jobname      = workunit_name( $workunit );

if( $user -> id != $job -> user )
{
 $title  = "Job '".$jobname."' (".$workunitid.")";
 page_head( $title );
 exit_with_text( "You are not the owner of this job !" );
}

$config = get_config();

$jobapplication = mysql_fetch_object( mysql_query( "SELECT * FROM app WHERE id=".$workunit -> appid ) );
$jobapplicationname = $jobapplication -> name;
$jobapplicationfriendlyname = $jobapplication -> user_friendly_name;
$jobfops = $workunit -> rsc_fpops_est;
$jobmem = $workunit -> rsc_memory_bound;
$jobdisk = $workunit -> rsc_disk_bound;
$jobstatusstring = workunit_status_string( $workunit );

$coloredjobstatusstring = $jobstatusstring;
if( $jobstatusstring == "running" ) $coloredjobstatusstring = "<font color='green'><b>".$jobstatusstring."</b></font>";
if( $jobstatusstring == "queued" ) $coloredjobstatusstring = "<font color='blue'><b>".$jobstatusstring."</b></font>";
if( $jobstatusstring == "ERROR" ) $coloredjobstatusstring = "<font color='red'><b>".$jobstatusstring."</b></font>";

$jobsubmittime = time_str( $workunit -> create_time );

$workunitidstring = "<a href=workunit.php?wuid=".$job -> workunit.">".$job -> workunit."</a>";

$jobinputurl = parse_element( $workunit -> xml_doc, "<file_info>" );
$jobinputurl = parse_element( $jobinputurl, "<url>" );
$jobinput = parse_element( $workunit -> xml_doc, "<file_info>" );
$jobinput = parse_element( $jobinput, "<name>" );

page_head( $title );
start_table();

row1( "Job speciffics" );
row2( "Job status: ", $coloredjobstatusstring );
row2( "Job application: ", $jobapplicationfriendlyname );
row2( "Job submit time: ", $jobsubmittime );
row2( "Job name: ", $jobname );
row2( "Job id: ", $workunitidstring );
row2( "Job estimated time to complete: ", floor((float)($jobfops)/92254963740)." min. " );
row2( "Job estimated memory usage: ", floor((float)($jobmem)/1048576)." Mb. " );
row2( "Job estimated disk usage: ", floor((float)($jobdisk)/1048576)." Mb. " );
row2( "Job input file:", '<a href="'.$jobinputurl.'"> '.$jobinput.'</a>' );

if( ( $jobstatusstring == "finished" ) || ( $jobstatusstring == "ERROR" ) )
{
 if( $jobstatusstring != "finished" )
 {
  $resultunitquery = mysql_query( "SELECT * FROM result WHERE workunitid=".$workunit -> id );
  $nrofresults = mysql_num_rows( $resultunitquery );

  for( $index = 0; $index < $nrofresults; ++$index )
  {
   $resultunit = mysql_fetch_object( $resultunitquery );

   $jobstderr = parse_element( $resultunit -> stderr_out, "<stderr_txt>" );

   if( $jobstderr )
   {
    row1( "Error output of this job" );
    row2( "", $jobstderr );
   }
  }
 }
 else
 {
  $resultunit = mysql_fetch_object( mysql_query( "SELECT * FROM result WHERE id=".$workunit -> canonical_resultid ) );

  $xmldoc = $resultunit -> xml_doc_out;
  $jobstderr = parse_element( $resultunit -> stderr_out, "<stderr_txt>" );

  $nroffiles = 0;
  $cursor = 0;
  while( $tempfileinfo = parse_next_element( $xmldoc, "<file_info>", &$cursor ) )
   $outputfiles[ $nroffiles++ ] = parse_element( $tempfileinfo, "<name>" );

  if( $nroffiles >= 1 )
  {
   $fanoutnr = parse_config( $config, "<uldl_dir_fanout>" );
   row1( "Output of this job" );
   row2( "Number of output files of job: ", $nroffiles );
   for( $index = 0; $index < $nroffiles; ++$index )
   { 
    $filename = $outputfiles[ $index ];
    $url = "upload/".fan_out_dir( $filename, $fanoutnr )."/".$filename;
    $outputfilelink = '<a href="'.$url.'">'.$filename.'</a>';
    row2( "Output file ".($index+1).": ", $outputfilelink );
   }
  }

  if( $jobstderr )
  {
   row1( "Error output of this job" );
   row2( "", $jobstderr );
  }
 }
}

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
row2( "", '<a href="queue_remove_job.php?workunitid='.$workunit -> id.'">Kill or Remove this job</a>' );
row2( "", '<a href="queue_show_queue.php">Go back to your queue</a>' );
row2( "", '<a href="logout.php">Log out</a>' );

end_table();

page_tail(); 
?>
