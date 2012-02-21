<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// web interface for creating an app or app version
//
// contributor: Natalia Nikitina

require_once("../inc/util.inc");

function handle_create_form() {
    global $project, $auth;

    page_head("Create app");
    echo "
        This form lets you specify parameters for a new application.

        <p>
        <form action=submit_example_app.php>
        <input type=hidden name=action value=create_action>
    ";
    start_table();
    row2("Application name", "<input name=app_name value=\"enter name\">");
    row2("Replication level", "<input name=replication_level value=1>");
    row2("Beta flag", "<input name=is_beta type=checkbox checked=1>");
    row2("Validator", "<select name=validator_type><option value=1 selected=1>Trivial</option>
                       <option value=2>Bitwise</option></select>");
    end_table();

    start_table();
    row2("Version number", "<input name=app_version value=\"1.0\">");

    //Get list of registered platforms

    //---
    //(tested on a local server)
    $link = mysql_connect("localhost","boincadm","") or die("Could not connect: " . mysql_error());
    $q = mysql_query("use test24");
    $q = mysql_query("select id,name from platform order by id");
    $options_platforms = "";
    while($f=mysql_fetch_row($q)) $options_platforms .= "<option value=$f[0]>$f[1]</option>";
    //---

    row2("Platform", "<select name=app_platform>".$options_platforms."</select>");
    row2("Plan class", "<input name=plan_class value=\"(none)\">");
    row2("Main program", "<input name=main_program type=file> <br/>
                          <input name=is_boincapp type=checkbox checked> Native BOINC application");


    if(isset($_GET['add_file']))
        $add_file = $_GET['add_file']+1;
    else
        $add_file = 1;
    for($i=0; $i<$add_file; $i++)
        row2("Additional file", "<input name=additional_file_$i type=file><br/>
                                 <input name=is_copyfile_$i type=checkbox checked>Copyfile");

    row2("","<a href=?action=create_form&add_file=$add_file>(more additional files)</a>");
    row2("","<input type=submit name=submit value=Submit>");
    end_table();
    echo "</form>\n";
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

handle_create_form();

?>
