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

// show detailed info about the tables in project's DB

require_once("../inc/util_ops.inc");

function showTableStatus() {
    $size = 0;
    $out = "";
    start_table();
    row_array(array("Name", "Engine", "Version", "Row Format", "Rows", "Avg Row Length (KB)", "Data Length (MB)", "Max Data Length (MB)", "Index Length (MB)", "Data free (MB)", "Create Time", "Update Time", "Check Time", "Create Options", "Comment"));
    db_init();
    $result = _mysql_query("show table status");
    while($row = _mysql_fetch_array($result)) {
        $size += ($row["Data_length"] + $row["Index_length"]);
        $engine = $row["Engine"];
        //if (!$engine) $engine = $row["Type"];
        row_array(array(
            $row["Name"],
            $engine,
            $row["Version"] ,
            $row["Row_format"] ,
            $row["Rows"] ,
            round($row["Avg_row_length"]/1024,2) ,
            round($row["Data_length"]/(1024*1024),2) ,
            round($row["Max_data_length"]/(1024*1024),2) ,
            round($row["Index_length"]/(1024*1024),2) ,
            round($row["Data_free"]/(1024*1024),2) ,
            $row["Create_time"] ,
            $row["Update_time"] ,
            $row["Check_time"] ,
            $row["Create_options"] ,
            $row["Comment"]
        ));
    }
    $size = round(($size/1024)/1024, 1);
    row2("Total Table Sizes (MB)", $size);
    end_table();
    echo "<BR><BR>";
}

db_init();
admin_page_head("MySQL Table Stats");

showTableStatus();
admin_page_tail();
?>

