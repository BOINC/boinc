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

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");
require_once('../inc/sanitize_html.inc');

db_init();

set_time_limit(0);

function image_as_bb($text){
    // This function depends on sanitized HTML

    $pattern = '@<img(.*) src=\"([^>^"]+)\"([^>]*)>@si';
    $replacement = '[img]$2[/img]';
    $text = preg_replace($pattern, $replacement, $text);

    $pattern = "@<img(.*) src='([^>^']+)'([^>]*)>@si";
    $replacement = '[img]$2[/img]';
    $text = preg_replace($pattern, $replacement, $text);

    return $text;
}

function link_as_bb($text){
        /* This function depends on sanitized HTML */
    // Build some regex (should be a *lot* faster)
    $pattern = '@<a href=\"([^>]+)\">@si'; // Gives us the URL in $1...
    $replacement = '[url=$1]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);
    $pattern = "@<a href='([^>]+)'>@si"; // Gives us the URL in $1...
    $replacement = '[url=$1]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);

    $pattern = "@</a>@si";
    $replacement = '[/url]';
    $text = preg_replace($pattern, $replacement, $text);
    return $text;
}

function formatting_as_bb($text){
        /* This function depends on sanitized HTML */
    $in[]="<b>";$out[]="[b]";
    $in[]="</b>";$out[]="[/b]";

    $in[]="<i>";$out[]="[i]";
    $in[]="</i>";$out[]="[/i]";

    $in[]="<u>";$out[]="[u]";
    $in[]="</u>";$out[]="[/u]";

    $in[]="<b>";$out[]="[b]";
    $in[]="</b>";$out[]="[/b]";

    $in[]="<ul>";$out[]="[list]";
    $in[]="</ul>";$out[]="[/list]";

    $in[]="<ol>";$out[]="[list=1]";
    $in[]="</ol>";$out[]="[/list]";

    $in[]="<pre>";$out[]="[pre]";
    $in[]="</pre>";$out[]="[/pre]";

    $in[]="</br>";$out[]="\n";
    $in[]="<br/>";$out[]="\n";
    $in[]="<br>";$out[]="\n";
    $in[]="&gt;";$out[]=">";
    $in[]="&lt;";$out[]="<";
    $in[]="&amp;";$out[]="&";

    return str_replace($in, $out, $text);
}

function fix_text($text) {
    $text = sanitize_html($text);    
    $text = image_as_bb($text);
    $text = link_as_bb($text);
    $text = formatting_as_bb($text);
    return $text;
}

function fix_profile($profile) {
    $text = fix_text($profile->response1);
    if ($text != $profile->response1) {
        $query = "update profile set response1 = '".mysql_escape_string($text)."' where userid=".$profile->userid;
        //echo "$profile->response1\n\n";
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
