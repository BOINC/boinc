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
require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");
require_once('../inc/sanitize_html.inc');
require_once('../inc/bbcode_convert.inc');

db_init();

set_time_limit(0);

function fix_post($post) {
    $text = html_to_bbcode($post->content);
    if ($text != $post->content) {
        $query = "update post set content = '".mysql_escape_string($text)."' where id=".$post->id;
        //echo "$post->content\n\n";
        //echo "$post->thread $query\n\n";
        $retval = mysql_query($query);
        if (!$retval) {
            echo mysql_error();
            exit();
        }
    }
}

function fix_posts() {
    $start_id = 0; //Set this to something else if you like
    $posts = mysql_query("select * from post where id>$start_id order by id");
    echo mysql_error();
    $i=0;
    while ($post = mysql_fetch_object($posts)){
        $i++; 
        if ($i%100 == 0) {                      //For every 100 posts
            echo $post->id.". "; flush();   // print out where we are
            //usleep(200000);
        }
        
        if ($post->id > $start_id){
            fix_post($post);
        }
    }
}

// use this to patch problem cases; hand-edit
function fix_fix() {
    $posts = mysql_query("select * from post where id=99");
    $post = mysql_fetch_object($posts);
    fix_post($post);
}

fix_posts();
//fix_fix();

?>
