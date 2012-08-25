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



//  Display and Manage BOINC Application Versions
//
// This page presents a form with information about application versions.
// Some of the fields can be changed.
//
// Eric Myers <myers@spy-hill.net>  - 4 June 2006
// @(#) $Id$

// TODO - code cleanup and use new DB interface

require_once('../inc/util_ops.inc');

db_init();

$commands = "";

// process form input for changes
//
function do_updates() {
    $apps = BoincApp::enum("");

    foreach ($apps as $app) {
        $id = $app->id;

        // Change deprecated status?
        //
        $field = "deprecated_".$id;
        $new_v = (post_str($field, true)=='on') ? 1 : 0;
        $old_v = $app->deprecated;
        if ($new_v != $old_v ) {
            $app->update("deprecated=$new_v");
        }

        $field = "weight_".$id;
        $new_v = $_POST[$field] + 0;
        $old_v = $app->weight;
        if ($new_v != $old_v ) {
            $app->update("weight=$new_v");
        }

        $field = "homogeneous_redundancy_".$id;
        $new_v = $_POST[$field];
        $old_v = $app->homogeneous_redundancy;
        if ($new_v != $old_v ) {
            $app->update("homogeneous_redundancy=$new_v");
        }

        $field = "homogeneous_app_version_".$id;
        $new_v = (post_str($field, true)=='on') ? 1 : 0;
        $old_v = $app->homogeneous_app_version;
        if ($new_v != $old_v ) {
            $app->update("homogeneous_app_version=$new_v");
        }

        $field = "non_cpu_intensive_".$id;
        $new_v = (post_str($field, true)=='on') ? 1 : 0;
        $old_v = $app->non_cpu_intensive;
        if ($new_v != $old_v ) {
            $app->update("non_cpu_intensive=$new_v");
        }
    }

    // Adding a new application

    if (post_str('add_app', true)) {
        $name = mysql_real_escape_string($_POST['add_name']);
        $user_friendly_name = mysql_real_escape_string($_POST['add_user_friendly_name']);
        if (empty($name) || empty($user_friendly_name) ) {
            $commands .= "<p><font color='red'>
                To add a new application please supply both a brief name and a
                longer 'user-friendly' name.</font></p>
            ";
        } else {
            $now = time();
            $cmd =  "INSERT INTO app (name,user_friendly_name,create_time) ".
                "VALUES ('$name', '$user_friendly_name',$now)";
            $commands .= "<P><pre>$cmd</pre>\n";
            mysql_query($cmd);
        }
    }
}


function show_form($updated) {
    admin_page_head("Manage Applications");

    if ($updated) {
        echo "Updates were done.
            <p>
            <b>You must stop and restart the project
            for these changes to take effect</b>.
        ";
    }

    $self=$_SERVER['PHP_SELF'];
    echo "
        <h2>Edit applications</h2>
        <form action='$self' method='POST'>
    ";

    start_table();
    table_header(
        "ID",
        "Name and description<br><span class=note>Click for details</span>",
        "Created",
        "weight<br><a href=http://boinc.berkeley.edu/trac/wiki/BackendPrograms#feeder><span class=note>details</span></a>",
        "homogeneous redundancy type<br><a href=http://boinc.berkeley.edu/trac/wiki/HomogeneousRedundancy><span class=note>details</span></a>",
        "homogeneous app version?<br><a href=http://boinc.berkeley.edu/trac/wiki/HomogeneousAppVersion><span class=note>details</span></a>",
        "deprecated?",
        "Non-CPU-intensive?"
    );

    $total_weight = mysql_query('SELECT SUM(weight) AS total_weight FROM app WHERE deprecated=0');
    $total_weight = mysql_fetch_assoc($total_weight);
    $total_weight = $total_weight['total_weight'];

    $q="SELECT * FROM app ORDER BY id";
    $result = mysql_query($q);
    $Nrow=mysql_num_rows($result);
    for ($j=1; $j<=$Nrow; $j++){
        $item = mysql_fetch_object($result);
        $id = $item->id;

        // grey-out deprecated versions
        $f1=$f2='';
        if($item->deprecated==1) {
            $f1 = "<font color='GREY'>";
            $f2 = "</font>";
        }
        echo "<tr> ";
        echo "  <TD align='center'>$f1 $id $f2</TD>\n";

        $name = $item->name;
        $full_name = $item->user_friendly_name;
        echo "  <TD align='left'>$f1<a href=app_details.php?appid=$id>$name</a><br> $full_name $f2</TD>\n";

        $time = $item->create_time;
        echo "  <TD align='center'>$f1 " .date_str($time)."$f2</TD>\n";

        $field = "weight_".$id;
        $v = $item->weight;
        echo "  <TD align='center'>
        <input type='text' size='4' name='$field' value='$v'></TD>\n";

        $field = "homogeneous_redundancy_".$id;
        $v = $item->homogeneous_redundancy;
        echo "  <TD align='center'>
            <input name='$field' value='$v'></TD>
        ";

        $field = "homogeneous_app_version_".$id;
        $v = '';
        if ($item->homogeneous_app_version) $v=' CHECKED ';
        echo "  <TD align='center'>
            <input name='$field' type='checkbox' $v></TD>
        ";

        $field = "deprecated_".$id;
        $v = '';
        if ($item->deprecated) $v = ' CHECKED ';
        echo "  <TD align='center'>
            <input name='$field' type='checkbox' $v></TD>
        ";

        $field = "non_cpu_intensive_".$id;
        $v = '';
        if ($item->non_cpu_intensive) $v = ' CHECKED ';
        echo "  <TD align='center'>
            <input name='$field' type='checkbox' $v></TD>
        ";

        echo "</tr> ";
    }
    mysql_free_result($result);
    echo "<tr><td colspan=7></td><td><input type='submit' name='update' value='Update'></td></tr>";

    end_table();


    // Entry form to create a new application
    //

    echo"<P>
         <h2>Add an application</h2>
      To add an application enter the short name and description
      ('user friendly name') below.  You can then edit the
      application when it appears in the table above.
     </p>\n";

    start_table("align='center' ");

    echo "<TR><TH>Name</TH>
              <TH>Description</TH>
              <TH> &nbsp;   </TH>
          </TR>\n";


    echo "<TR>
            <TD> <input type='text' size='12' name='add_name' value=''></TD>
            <TD> <input type='text' size='35' name='add_user_friendly_name' value=''></TD>
            <TD align='center' >
                 <input type='submit' name='add_app' value='Add Application'></TD>
            </TR>\n";

    end_table();
    echo "</form><p>\n";
    admin_page_tail();
}

if( !empty($_POST) ) {
    do_updates();
    show_form(true);
} else {
    show_form(false);
}

//Generated automatically - do not edit
$cvs_version_tracker[]="\$Id$";
?>
