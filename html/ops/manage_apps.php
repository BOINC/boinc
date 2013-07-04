<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// web interface for managing apps

require_once('../inc/util_ops.inc');

function do_updates() {
    $id = post_int("id");
    $app = BoincApp::lookup_id($id);
    if (!$app) error_page("no such app");

    $n = post_str("deprecated", true)?1:0;
    $app->update("deprecated=$n");

    $n = post_num("weight");
    $app->update("weight=$n");

    $n = post_int("homogeneous_redundancy");
    $app->update("homogeneous_redundancy=$n");

    $n = post_str("homogeneous_app_version", true)?1:0;
    $app->update("homogeneous_app_version=$n");

    $n = post_str("cpu_intensive", true)?1:0;
    $app->update("non_cpu_intensive=$n");

    $n = post_str("beta", true)?1:0;
    $app->update("beta=$n");

    echo "Application $id updated.
        <p>
        You must restart the project for this to take effect.
    ";
}

function add_app() {
    $name = mysql_real_escape_string(post_str('add_name'));
    $user_friendly_name = mysql_real_escape_string(post_str('add_user_friendly_name'));
    if (empty($name) || empty($user_friendly_name) ) {
        error_page(
            "To add a new application please supply both a brief name and a longer 'user-friendly' name.</font></p>"
        );
    }
    $now = time();
    $id = BoincApp::insert(
        "(name,user_friendly_name,create_time) VALUES ('$name', '$user_friendly_name', $now)"
    );
    if (!$id) {
        error_page("insert failed");
    }
    echo "Application added.
        <p>
        You must restart the project for this to take effect.
    ";
}

function show_form() {
    echo "
        <h2>Edit applications</h2>
    ";

    start_table();
    table_header(
        "ID",
        "Name and description<br><span class=note>Click for details</span>",
        "Created",
        "weight<br><a href=http://boinc.berkeley.edu/trac/wiki/BackendPrograms#feeder><span class=note>details</span></a>",
        "shmem items",
        "homogeneous redundancy type<br><a href=http://boinc.berkeley.edu/trac/wiki/HomogeneousRedundancy><span class=note>details</span></a>",
        "homogeneous app version?<br><a href=http://boinc.berkeley.edu/trac/wiki/HomogeneousAppVersion><span class=note>details</span></a>",
        "deprecated?",
        "Non-CPU-intensive?",
        "Beta?"
    );

    $total_weight = BoincApp::sum("weight");
    $swi = parse_config(get_config(), "<shmem_work_items>");
    if (!$swi) {
        $swi = 100;
    }

    $apps = BoincApp::enum("");
    foreach ($apps as $app) {
        // grey-out deprecated versions
        $f1=$f2='';
        if ($app->deprecated==1) {
            $f1 = "<font color='GREY'>";
            $f2 = "</font>";
        }
        echo "<tr><form action=manage_apps.php method=POST>";
        echo "<input type=hidden name=id value=$app->id>";
        echo "  <TD align='center'>$f1 $app->id $f2</TD>\n";

        echo "  <TD align='left'>$f1<a href=app_details.php?appid=$app->id>$app->name</a><br> $app->user_friendly_name $f2</TD>\n";

        echo "  <TD align='center'>$f1 " .date_str($app->create_time)."$f2</TD>\n";

        $v = $app->weight;
        echo "  <TD align='center'>
        <input type='text' size='4' name='weight' value='$v'></TD>\n";
        
        if ($app->deprecated || ($total_weight == 0)) {
            echo '<td></td>';
        } else {
            echo '<td align="right">'.round($app->weight/$total_weight*$swi).'</td>';
        }

        $v = $app->homogeneous_redundancy;
        echo "  <TD align='center'>
            <input name='homogeneous_redundancy' value='$v'></TD>
        ";

        $v = '';
        if ($app->homogeneous_app_version) $v=' CHECKED ';
        echo "  <TD align='center'>
            <input name='homogeneous_app_version' type='checkbox' $v></TD>
        ";

        $v = '';
        if ($app->deprecated) $v = ' CHECKED ';
        echo "  <TD align='center'>
            <input name='deprecated' type='checkbox' $v></TD>
        ";

        $v = '';
        if ($app->non_cpu_intensive) $v = ' CHECKED ';
        echo "  <TD align='center'>
            <input name='non_cpu_intensive' type='checkbox' $v></TD>
        ";

        $v = '';
        if ($app->beta) $v = ' CHECKED ';
        echo "  <TD align='center'>
            <input name='beta' type='checkbox' $v></TD>
        ";

        echo "<td><input type=submit name=submit value=Update>";
        echo "</tr></form>";
    }

    end_table();


    // Entry form to create a new application
    //

    echo"<P>
        <h2>Add an application</h2>
        To add an application enter the short name and description
        ('user friendly name') below.  You can then edit the
        application when it appears in the table above.
        <p>
        <form action=manage_apps.php method=POST>
    ";

    start_table("align='center' ");

    table_header("Name", "Description", "&nbsp;");

    echo "<TR>
            <TD> <input type='text' size='12' name='add_name' value=''></TD>
            <TD> <input type='text' size='35' name='add_user_friendly_name' value=''></TD>
            <TD align='center' >
                 <input type='submit' name='add_app' value='Add Application'></TD>
            </TR>\n";

    end_table();
    echo "</form><p>\n";
}

admin_page_head("Manage applications");

if (post_str('add_app', true)) {
    add_app();
} else if (post_str('submit', true)) {
    do_updates();
}
show_form(false);
admin_page_tail();

?>
