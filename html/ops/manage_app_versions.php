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
 * Some of the fields can be changed.
 *
 * Eric Myers <myers@spy-hill.net>  - 4 June 2006
 * @(#) $Id$
\***********************************************************************/

// TODO: rewrite this using the new DB interface

require_once('../inc/util_ops.inc');

db_init();

// Platform and application labels (are better than numbers)

$result = mysql_query("SELECT * FROM platform");
$Nplatform =  mysql_num_rows($result);
for($i=0;$i<$Nplatform;$i++){
    $item=mysql_fetch_object($result);
    $id=$item->id;
    $plat_off[$id]=$item->deprecated; 
    $platform[$id]=$item->user_friendly_name;
}
mysql_free_result($result);


$result = mysql_query("SELECT * FROM app");
$Napp =  mysql_num_rows($result);
for($i=0;$i<$Napp;$i++){
    $item=mysql_fetch_object($result);
    $id=$item->id;
    $app_off[$id]=$item->deprecated; 
    $app[$id]=$item->name;
}
mysql_free_result($result);

$commands="";

/***************************************************\
 *  Action: process form input for changes
 \***************************************************/

if( !empty($_POST) ) {

    $result = mysql_query("SELECT * FROM app_version");
    $Nrow=mysql_num_rows($result);

    for($j=1;$j<=$Nrow;$j++){  // test/update each row in DB
        $item=mysql_fetch_object($result);
        $id=$item->id;

        /* Change deprecated status? */
        $field="deprecated_".$id;
        $new_v= array_key_exists($field, $_POST) ? 1 : 0;
        $old_v=$item->deprecated;
        if ($new_v != $old_v ) {
            $cmd =  "UPDATE app_version SET deprecated=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }

        /* Minimum core version limit */
        $field="min_core_version_".$id;
        $new_v= post_int($field);
        $old_v=$item->min_core_version;
        if ($new_v != $old_v ) {
            $cmd =  "UPDATE app_version SET min_core_version=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }

        /* Maximum core version limit */
        $field="max_core_version_".$id;
        $new_v= post_int($field);
        $old_v=$item->max_core_version;
        if($new_v != $old_v ) {
            $cmd =  "UPDATE app_version SET max_core_version=$new_v WHERE id=$id";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }
    }
    mysql_free_result($result);
}


/***************************************************\
 * Display the DB contents in a form
 \***************************************************/

admin_page_head("Manage Application Versions");

if (strlen($commands)) {
    echo "The following updates were done: $commands
        <p>
        <b>You must stop and restart the project
        for these changes to take effect.</b>
";
}

$self=$_SERVER['PHP_SELF'];
echo "<form action='$self' method='POST'>\n";


// Application Version table:

start_table("align='center'");

echo "<TR><TH>ID #</TH>
      <TH align=left>Application</TH>
      <TH>Version</TH>
      <TH>Platform</TH>
      <TH>Plan Class</TH>
      <TH>minimum<br>core version</TH>
      <TH>maximum<br>core version</TH>
      <TH>deprecated?</TH>
       </TR>\n";

$q="SELECT * FROM app_version ORDER BY appid, platformid, plan_class, version_num";
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
    echo "  <TD>$f1 $id $f2</TD>\n";

    $i=$item->appid;
    echo "  <TD>$f1 $app[$i] $f2</TD>\n";

    echo "  <TD>$f1 $item->version_num $f2</TD>\n";

    $i=$item->platformid;
    echo "  <TD>$f1 $platform[$i] $f2</TD>\n";

    echo "  <td>$f1 $item->plan_class $f2</td>\n";

    $field="min_core_version_".$id;
    $v=$item->min_core_version;
    echo "  <TD>
    <input type='text' size='4' name='$field' value='$v'></TD>\n";

    $field="max_core_version_".$id;
    $v=$item->max_core_version;
    echo "  <TD>
    <input type='text' size='4' name='$field' value='$v'></TD>\n";

    $field="deprecated_".$id;
    $v='';
    if($item->deprecated) $v=' CHECKED ';
    echo "  <TD>
    <input name='$field' type='checkbox' $v></TD>\n";

    echo "</tr> "; 
}
mysql_free_result($result);


echo "<tr><td colspan=7> </td>
    <td colspan=2 bgcolor='#FFFF88'>
    <input type='submit' value='Update'></td>
    </tr>
";

end_table();


echo "</form><P>\n";
admin_page_tail();

//Generated automatically - do not edit
$cvs_version_tracker[]="\$Id$"; 
?>
