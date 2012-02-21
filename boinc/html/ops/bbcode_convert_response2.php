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

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once('../inc/bbcode_convert.inc');

db_init();

set_time_limit(0);

function fix_profile($profile) {
    $text = html_to_bbcode($profile->response2);
    if ($text != $profile->response2) {
        $query = "update profile set response2 = '".mysql_escape_string($text)."' where userid=".$profile->userid;
        //echo "$profile->response2\n\n";
        //echo "$profile->thread $query\n\n";
        $retval = mysql_query($query);
        if (!$retval) {
            echo mysql_error();
            exit();
        }
    }
}

function fix_profiles() {
    $start_id = 0; //Set this to something else if you like
    $profiles = mysql_query("select * from profile where userid>$start_id order by userid");
    echo mysql_error();
    $i=0;
    while ($profile = mysql_fetch_object($profiles)){
        $i++; 
        if ($i%100 == 0) {                      //For every 100 profiles
            echo $profile->userid.". "; flush();   // print out where we are
            //usleep(200000);
        }
        
        if ($profile->userid > $start_id){
            fix_profile($profile);
        }
    }
}

// use this to patch problem cases; hand-edit
function fix_fix() {
    $profiles = mysql_query("select * from profile where id=99");
    $profile = mysql_fetch_object($profiles);
    fix_profile($profile);
}

fix_profiles();
//fix_fix();

?>
