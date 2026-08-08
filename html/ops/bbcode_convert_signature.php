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

function fix_forum_preferences($forum_preferences) {
    $text = html_to_bbcode($forum_preferences->signature);
    if ($text != $forum_preferences->signature) {
        $query = "update forum_preferences set signature = '"._mysql_escape_string($text)."' where userid=".$forum_preferences->userid;
        //echo "$forum_preferences->signature\n\n";
        //echo "$forum_preferences->thread $query\n\n";
        $retval = _mysql_query($query);
        if (!$retval) {
            echo _mysql_error();
            exit();
        }
    }
}

function fix_forum_preferencess() {
    $start_id = 0; //Set this to something else if you like
    $forum_preferencess = _mysql_query("select * from forum_preferences where userid>$start_id order by userid");
    echo _mysql_error();
    $i=0;
    while ($forum_preferences = _mysql_fetch_object($forum_preferencess)){
        $i++;
        if ($i%100 == 0) {                      //For every 100 forum_preferencess
            echo $forum_preferences->userid.". "; flush();   // print out where we are
            //usleep(200000);
        }

        if ($forum_preferences->userid > $start_id){
            fix_forum_preferences($forum_preferences);
        }
    }
}

// use this to patch problem cases; hand-edit
function fix_fix() {
    $forum_preferencess = _mysql_query("select * from forum_preferences where id=99");
    $forum_preferences = _mysql_fetch_object($forum_preferencess);
    fix_forum_preferences($forum_preferences);
}

fix_forum_preferencess();
//fix_fix();

?>
