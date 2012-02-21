<?php

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/prefs.inc" );
include_once( "../inc/queue.inc" );

db_init();

$user        = get_logged_in_user();
$timestr     = time_str(time(0));
$title       = "New job for ".$user -> name." at ".$timestr;

page_head( $title );

$appresult = mysql_query( "SELECT * FROM app" );
$nrofapps = mysql_num_rows( $appresult );

if( $nrofapps )
{

 $selection = "";
 for( $appindex = 0; $appindex < $nrofapps; ++$appindex )
 {
  $app = mysql_fetch_object( $appresult );

  $appqmax = nr_of_jobs_for_user_for_app( $user, $app );
  $appsubmitted = nr_of_submitted_jobs_for_user_for_app( $user, $app );

  if( $appqmax > $appsubmitted )
   $selection = $selection.'<option value="'.$app -> id.'">'.$app -> user_friendly_name.'</option>';
 }

 if( $selection != "" )
 {

  echo '<form action="queue_new_job_form_action.php" method="POST">';
  start_table();

  row1( "New job speciffics" );
  $selection = "<select name=\"application\">".$selection."</select>";
  row2( "Application: ", $selection );
  row2( "Name of job (no spaces, quotes or slashes): ", '<input type="text" size="65" name="name" value="" maxlength="128" >' );
  row2( "Input: ", '<textarea wrap="off" rows="20" cols="74" name="input"></textarea>' );

  // These numbers have been based on running our app on our pool of 10000 desktops at that time
  // for about 10000 WU's. For each successfull result the 'cpu_time' was multiplied with the hosts
  // 'p_fpops' to get an estimate for the WU number of fops (which sould be fairly constant). This
  // number was the devided by the average of all hosts 'cpu_time'. If your pool is bigger/different
  // you might check these numbers first with your app, but they should be fairly okay...

  $selection = '<option value="461274818700"> 5 min. </option>';
  $selection .= '<option value="1383824456100"> 15 min. </option>';
  $selection .= '<option value="2767648912200"> 30 min. </option>';
  $selection .= '<option value="5535297824400"> 1 h. </option>';
  $selection .= '<option value="16605893473200"> 3 h. </option>';
  $selection .= '<option value="33211786946400"> 6 h. </option>';
  $selection = "<select name=\"fops\">".$selection."</select>";
  row2( "Estimated time to completion: ", $selection );

  $selection = '<option value="2097152"> 2 Mb. </option>';
  $selection .= '<option value="4194304"> 4 Mb. </option>';
  $selection .= '<option value="16777216"> 16 Mb. </option>';
  $selection .= '<option value="67108864"> 64 Mb. </option>';
  $selection .= '<option value="134217728"> 128 Mb. </option>';
  $selection = "<select name=\"mem\">".$selection."</select>";
  row2( "Estimated memory usage: ", $selection );
  
  $selection = '<option value="2097152"> 2 Mb. </option>';
  $selection .= '<option value="4194304"> 4 Mb. </option>';
  $selection .= '<option value="16777216"> 16 Mb. </option>';
  $selection .= '<option value="67108864"> 64 Mb. </option>';
  $selection .= '<option value="134217728"> 128 Mb. </option>';
  $selection .= '<option value="536870912"> 512 Mb. </option>';
  $selection = "<select name=\"disk\">".$selection."</select>";
  row2( "Estimated disk usage: ", $selection );

  row2( "", '<input type="submit" value="     Submit Job     ">' );

  row1( "Commands" );
  row2( "", '<a href="queue_show_queue.php">Go back to your queue</a>' );
  row2( "", '<a href="logout.php">Log out</a>' );

  end_table();
  echo '</form>';

 }
 else
  exit_with_text( "You are not allowed to submit any jobs !" );

}

page_tail(); 

?>
