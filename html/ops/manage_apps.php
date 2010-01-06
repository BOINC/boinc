<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.



/***********************************************************************\
 *  Display and Manage BOINC Application Versions
 * 
 * This page presents a form with information about application versions.
 * Some of the fields can be changed.   An appllication version can be deleted
 * by entering the word "DELETE" (all caps required) in the provided field.   
 * It is better to deprecate a version first than to delete it, but it is also 
 * good to remove old versions after they have been unused for a while,
 * lest you over-fill the feeder (which results in new versions not being
 * used by clients).
 *
 * Eric Myers <myers@spy-hill.net>  - 4 June 2006
 * @(#) $Id$
\***********************************************************************/

require_once('../inc/util_ops.inc');

db_init();

// Platform and application labels (are better than numbers)

$result = mysql_query("SELECT * FROM platform");
$Nplatform =  mysql_num_rows($result);
for($i=0;$i<=$Nplatform;$i++){
    $item=mysql_fetch_object($result);
    $id=$item->id;
    $plat_off[$id]=$item->deprecated; 
    $platform[$id]=$item->user_friendly_name;
 }
mysql_free_result($result);


/***************************************************\
 *  Action: process form input for changes
 \***************************************************/

if( !empty($_POST) ) {

    /* Changing properties of existing applications */

    $result = mysql_query("SELECT * FROM app");
    $Nrow=mysql_num_rows($result);

    for($j=1;$j<=$Nrow;$j++){  // test/update each row in DB
        $item=mysql_fetch_object($result);
        $id=$item->id;

        /* Delete this entry? */
        $field="delete_".$id; 
        if( $_POST[$field]=='DELETE' ) {
            $cmd =  "DELETE FROM app WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
            continue;  // next row, this one is gone
        }

        /* Change deprecated status? */
        $field="deprecated_".$id;
        $new_v= ($_POST[$field]=='on') ? 1 : 0;
        $old_v=$item->deprecated;
        if($new_v != $old_v ) {
            $cmd =  "UPDATE app SET deprecated=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }

        /* Minimum version limit */
        $field="min_version_".$id;
        $new_v= $_POST[$field] + 0;
        $old_v=$item->min_version;
        if( $new_v != $old_v ) {
            $cmd =  "UPDATE app SET min_version=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }


        /* Minimum version limit */
        $field="weight_".$id;
        $new_v= $_POST[$field] + 0;
        $old_v=$item->weight;
        if( $new_v != $old_v ) {
            $cmd =  "UPDATE app SET weight=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }

        /* Homogendous redundancy restriction (same platform for all WU's) */
        $field="homogeneous_redundancy_".$id;
        $new_v= $_POST[$field];
        $old_v=$item->homogeneous_redundancy;
        if( $new_v != $old_v ) {
            $cmd =  "UPDATE app SET homogeneous_redundancy=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }
    }

    /* Adding a new application */

    if( $_POST['add_app'] ) {
        $name= mysql_real_escape_string($_POST['add_name']);
        $user_friendly_name=mysql_real_escape_string($_POST['add_user_friendly_name']);
        if( empty($name) || empty($user_friendly_name) ) {
            $commands .= "<p><font color='red'>
            To add a new application please supply both a brief name and a
                longer 'user-friendly' name.</font></p>\n";
        }
        else {
            $now=time();
            $cmd =  "INSERT INTO app (name,user_friendly_name,create_time) ".
                "VALUES ('$name', '$user_friendly_name',$now)";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }
    }
 }//$_POST


/***************************************************\
 * Display the DB contents in a form
 \***************************************************/

admin_page_head("Manage Project Applications");

if($commands) echo $commands;


$self=$_SERVER['PHP_SELF'];
echo "<form action='$self' method='POST'>\n";

// Application Table:

echo"<P>
     <h2>". PROJECT . " Applications</h2>\n";

start_table("align='center'");
echo "<TR><TH>ID #</TH>
      <TH>Name<br>(description)</TH>
      <TH>Creation<br>Time</TH>
      <TH>minimum<br>version</TH>
      <TH>weight</TH>
      <TH>homogeneous<br>redundancy<br>class (0=none)</TH>
      <TH>deprecated?</TH>
      <TH>DELETE?<sup>*</sup>
    </TH>
       </TR>\n";

$q="SELECT * FROM app ORDER BY id";
$result = mysql_query($q);
$Nrow=mysql_num_rows($result);
for($j=1;$j<=$Nrow;$j++){
    $item=mysql_fetch_object($result);
    $id=$item->id;

    // grey-out deprecated versions 
    $f1=$f2='';
    if($item->deprecated==1) {
        $f1="<font color='GREY'>";
        $f2="</font>";
    }
    echo "<tr> ";
    echo "  <TD align='center'>$f1 $id $f2</TD>\n";

    $name=$item->name;
    $full_name=$item->user_friendly_name;
    echo "  <TD align='left'>$f1<tt>$name</tt><br>
                <em>$full_name</em> $f2</TD>\n";

    $time=$item->create_time;
    echo "  <TD align='center'>$f1 " .time_str($time)."$f2</TD>\n";

    $field="min_version_".$id;
    $v=$item->min_version;
    echo "  <TD align='center'>
    <input type='text' size='4' name='$field' value='$v'></TD>\n";

    $field="weight_".$id;
    $v=$item->weight;
    echo "  <TD align='center'>
    <input type='text' size='4' name='$field' value='$v'></TD>\n";



    $field="homogeneous_redundancy_".$id;
    $v = $item->homogeneous_redundancy;
    echo "  <TD align='center'>
        <input name='$field' value='$v'></TD>
    ";

    $field="deprecated_".$id;
    $v='';
    if($item->deprecated) $v=' CHECKED ';
    echo "  <TD align='center'>
    <input name='$field' type='checkbox' $v></TD>\n";

    $field="delete_app_".$id; 
    echo "  <TD align='center'>
    <input type='text' size='6' name='$field' value=''></TD>\n";
    echo "</tr> "; 
 }
mysql_free_result($result);

echo "<tr><td colspan=6>
    <sup>*</sup>To delete an entry you must enter the word 'DELETE' in this field,
        in all capital letters..
          <td align='center' colspan=2 bgcolor='#FFFF88'>
              <input type='submit' name='update' value='Update'></td>
    </tr>\n";

end_table();


/**
 * Entry form to create a new application
 */

echo"<P>
     <h2>Add an Application</h2>
  To add an application to the project enter the short name and description 
  ('user friendly name') below.  You can then control the version limits and 
  turn on homogeneous redundancy (if desired) when the application appears
  in the table above.
 </p>\n";

start_table("align='center' ");

echo "<TR><TH>Name</TH>
          <TH>Description</TH>
          <TH> &nbsp;   </TH>
      </TR>\n";


echo "<TR>
        <TD> <input type='text' size='12' name='add_name' value=''></TD>
        <TD> <input type='text' size='35' name='add_user_friendly_name' value=''></TD>
        <TD align='center' bgcolor='#FFFF88'>
             <input type='submit' name='add_app' value='Add Application'></TD>
        </TR>\n";

end_table();

echo "</form><p>\n";

admin_page_tail();

//Generated automatically - do not edit
$cvs_version_tracker[]="\$Id$";
?>
