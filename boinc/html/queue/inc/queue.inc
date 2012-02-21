<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/prefs.inc");

function all_jobs_of_user( $user )
{
 $alljobs = mysql_query( "SELECT * FROM q_list WHERE user=".$user -> id );
 return $alljobs;
}

function nr_of_jobs_of_user( $user )
{
 $njobs = mysql_num_rows( all_jobs_of_user( $user ) );
 return $njobs;
}

function workunit_name( $workunit )
{
 if( ( $pos = strpos( $workunit -> name, "_queue" ) ) === false )
  $workunitname = $workunit -> name;
 else
  $workunitname = substr( $workunit -> name, 0, $pos );
 return $workunitname;
}

function workunit_status_string( $workunit )
{
 $status = "UNKNOWN";
 if( $workunit -> canonical_resultid )
  $status = "finished";
 else
 {
  if( $workunit -> hr_class )
   $status = "running";
  else
   $status = "queued";
 }
 if( $workunit -> error_mask )
 {
  $status = "ERROR";
  if( $workunit -> error_mask & 16 )
   $status = "CANCELED";
 }
 return $status;
}

function max_nr_of_jobs_of_user( $user )
{
 $allapps = mysql_query( "SELECT * FROM q_users WHERE user=".$user -> id );
 $napps   = mysql_num_rows( $allapps );

 if( $napps > 0 ) 
  for( $count = $index = 0; $index < $napps; ++$index )
  {
   $row = mysql_fetch_object( $allapps );
   if( $row )
    $count += $row -> qmax;
  }
 else
  $count = 5;

 mysql_free_result( $allapps );
 return $count;
}

function nr_of_jobs_for_user_for_app( $user, $app )
{
 $qmaxresult = mysql_query( "SELECT qmax FROM q_users WHERE user=".$user -> id." AND app=".$app -> id );
 if( mysql_num_rows( $qmaxresult ) < 1 )
 {
  $qrestrictedapps = mysql_query( "SELECT * FROM q_restricted_apps WHERE appid=".$app -> id );
  if( mysql_num_rows( $qrestrictedapps ) < 1 )
   $nr = 5;
  else
   $nr = 0;
  mysql_free_result( $qrestrictedapps );
 }
 else
 {
  $object = mysql_fetch_object( $qmaxresult );
  $nr = $object -> qmax;
 }
 mysql_free_result( $qmaxresult );
 return $nr; 
}

function nr_of_submitted_jobs_for_user_for_app( $user, $app )
{
 $alljobs = mysql_query( "SELECT * FROM q_list WHERE user=".$user -> id );
 $nrofjobs = mysql_num_rows( $alljobs );

 for( $nr = $index = 0; $index < $nrofjobs; ++$index )
 {
  $job = mysql_fetch_object( $alljobs );
  $workunit = mysql_fetch_object( mysql_query( "SELECT * FROM workunit WHERE id=".$job -> workunit ) );

  if( $workunit -> appid == $app -> id )
   $nr = $nr + 1;
 }

 mysql_free_result( $alljobs );
 return $nr;
}

function exit_with_text( $text )
{
 start_table();
 row1( "<font color='red'><b>".$text."</b></font>" );
 row1( "Commands" );
 row2( "", '<a href="queue_show_queue.php">Go back to your queue</a>' );
 row2( "", '<a href="logout.php">Log out</a>' );
 end_table();
 page_tail();
 exit;
}

function fan_out_dir( $filename, $fanoutnr )
{
 $dir = dechex( hexdec( substr( md5( $filename ), 1, 7 ) ) % $fanoutnr );
 return $dir; 
}

function row5($xx, $xy, $yx, $yy, $zz ) {
    echo "<tr><td width=20% valign=top>$xx</td><td width=20%>$xy</td>"
        . "<td width=20% >$yx</td><td width=%20>$yy</td><td width=20%>$zz</td></tr>
    ";
}

function row6($xx, $xy, $yx, $yy, $zz, $xz ) {
    echo "<tr><td width=15% valign=top>$xx</td><td width=15%>$xy</td>"
        . "<td width=15% >$yx</td><td width=%15>$yy</td><td width=15%>$zz</td>"
        . "<td width=15%>$xz</td></tr>";
}

function remove_tags( $xml, $tag )
{
 $newxml = $xml;
 while( ( $pos = strpos( $newxml, $tag ) ) !== false )
  $newxml = substr( $newxml, 0, $pos ).substr( $newxml, $pos + strlen( $tag ) ); 
 return $newxml;
}

?>
