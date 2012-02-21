<?php

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/prefs.inc" );
include_once( "../inc/queue.inc" );

db_init();

$timestr           = time_str(time(0));

$jobapplication    = post_int( 'application' ); 
$jobname           = escapeshellcmd( $_POST[ 'name' ] );
$jobinput          = post_str( 'input' );
$jobfops           = post_int( 'fops' );
$jobdisk           = post_int( 'disk' );
$jobmem            = post_int( 'mem' );

if( get_magic_quotes_gpc() )
 $jobinput = stripslashes( $jobinput );

$config = get_config();
$name   = parse_config( $config, "<long_name>" );
$user   = get_logged_in_user();

$jobapplicationname = mysql_fetch_object( mysql_query( "SELECT * FROM app WHERE id=".$jobapplication ) );
$app = $jobapplicationname;
$jobapplicationfriendlyname = $jobapplicationname -> user_friendly_name;
$jobapplicationname = $jobapplicationname -> name;

$title       = "New job for '".$jobname."' ".$user -> name." at ".$timestr;
page_head( $title );

start_table();
row1( "Job speciffics" );
row2( "Job application: ", $jobapplicationfriendlyname );
row2( "Job name: ", $jobname );
row2( "Job estimated time to complete: ", floor( ( float )( $jobfops ) / 92254963740 )." min. " );
row2( "Job estimated memory usage: ", floor( ( float )( $jobmem ) / 1048576 )." Mb. " );
row2( "Job estimated disk usage: ", floor( ( float )( $jobdisk ) / 1048576 )." Mb. " );
row2( "Job input:", '<textarea wrap="off" rows="20" cols="74">'.$jobinput.'</textarea>' );
end_table();

if( ( $jobname == "" ) || strpos( $jobname, "queue" ) || strpos( $jobname, " " ) || 
    strpos( $jobname, '"' ) || strpos( $jobname, "'" ) || strpos( $jobname, "`" ) || 
    strpos( $jobname, "\\" ) )
 exit_with_text( "The job name is invalid !" );

if( $jobinput == "" )
 exit_with_text( "There was no input !" );

$appqmax = nr_of_jobs_for_user_for_app( $user, $app );
$appsubmitted = nr_of_submitted_jobs_for_user_for_app( $user, $app );
if( $appqmax <= $appsubmitted )
 exit_with_text( "Job limit would be exceeded!" );

$bin_dir = parse_config( $config, "<bin_dir>" );
$download_dir = parse_config( $config, "<download_dir>" );
$upload_dir = parse_config( $config, "<upload_dir>" );
$template_dir = parse_config( $config, "<template_dir>" );
$config_dir = parse_config( $config, "<project_dir>" );
$createworkprogram = parse_config( $config, "<create_work_program>" );

$extendedjobname = $jobname."_queue_".$jobapplication."_".time(0)."_".random_string();
$extendedjobname = escapeshellcmd( $extendedjobname );

$wu_template = $template_dir."/queue_".$jobapplicationname."_work_unit_template";
$result_template = $template_dir."/queue_".$jobapplicationname."_result_unit_template";
$temporaryinputfile = $extendedjobname;

$command_to_submit = $bin_dir."/".$createworkprogram;
$command_to_submit .= " -config_dir ".$config_dir;
$command_to_submit .= " -appname ".$jobapplicationname;
$command_to_submit .= " -wu_name ".$extendedjobname;
$command_to_submit .= " -wu_template ".$wu_template;
$command_to_submit .= " -result_template ".$result_template;
$command_to_submit .= " -rsc_fpops_est ".floor( ( float )( $jobfops ) );
$command_to_submit .= " -rsc_fpops_bound ".floor( 3.0 * ( float )( $jobfops ) );
$command_to_submit .= " -rsc_memory_bound ".floor( ( float )( $jobmem ) );
$command_to_submit .= " -rsc_disk_bound ".floor( ( float )( $jobdisk ) );
$command_to_submit .= " -priority 10 -batch ".$user -> id;
$command_to_submit .= " ".$temporaryinputfile;
$command_to_submit = escapeshellcmd( $command_to_submit );
$command_to_submit = "cd ".$config_dir."; ".$command_to_submit;

$temporaryinputfile = $download_dir."/".$temporaryinputfile;

$filehandle = fopen( $temporaryinputfile, "w" );
if( !$filehandle )
 exit_with_text( "Cannot create the temporary input file !" );

if( !fwrite( $filehandle, $jobinput ) )
{
 fclose( $filehandle );
 exit_with_text( "Cannot write to the temporary input file !" );
} 

fclose( $filehandle );

// We at Leiden Classical have a special mode for our app, it can verify the user supplied 
// input and return an exit code signaling an error. We tend to parse the input before let
// it run on our desktop pool ;-)... Here's how...
//
// if( strpos( $jobapplicationname, "classical" ) !== false )
// {
//  $testinputcommand = $bin_dir."/verify_classical_input ".$temporaryinputfile." /dev/null /dev/stdout /dev/stdout";
//  $testinputcommand = escapeshellcmd( $testinputcommand );
//  $testinputcommand = "cd ".$config_dir."; ".$testinputcommand;
//  $errorline = 0;
//  exec( $testinputcommand, &$outputoftest, &$errorline );
//  if( $errorline != 0 )
//  {
//   $errorstring = "Your input had an error on line ".$errorline." ! The job was not submitted !";
//   unlink( $temporaryinputfile );
//   exit_with_text( $errorstring ); 
//  }
// }

system( $command_to_submit );

unlink( $temporaryinputfile );

$workunit = mysql_fetch_object( mysql_query( "SELECT * FROM workunit WHERE name='".$extendedjobname."'" ) );

if( !$workunit )
 exit_with_text( "Error during submition of the workunit associated with your job !" );

$qlistentry = mysql_query( "INSERT INTO q_list VALUES('','".$user->id."','".$workunit->id."')" );

if( !$qlistentry )
 exit_with_text( "Error during submition of your job !" );

$jobidlink = '<a href="queue_show_job.php?workunitid='.$workunit -> id.'">'.$jobname.' ('.$workunit -> id.')</a>';

start_table();
row1( "Your job has been submitted !" );
row2( "Job status: ", workunit_status_string( $workunit ) );
row2( "Job id: ", $jobidlink );

row1( "Commands" );
row2( "Status of this job: ", '<a href="queue_show_job.php?workunitid='.$workunit -> id.'">Show job status</a>' );
$max_jobs = max_nr_of_jobs_of_user( $user );
$njobs = nr_of_jobs_of_user( $user );
if( $njobs < $max_jobs )
{
 if( $max_jobs - $njobs > 1 )
  $line = "You can submit ".($max_jobs-$njobs)." more jobs: ";
 else
  $line = "You can submit one more job: ";
 row2( $line, '<a href="queue_new_job_form.php">Submit another job</a>' );
}
row2( "", '<a href="queue_show_queue.php">Go back to your queue</a>' );
row2( "", '<a href="logout.php">Log out</a>' );

end_table();
page_tail(); 

?>
