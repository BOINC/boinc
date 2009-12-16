<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

require_once('../inc/sanitize_html.inc');

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

function html_to_bbcode($text) {
    $text = sanitize_html($text);    
    $text = image_as_bb($text);
    $text = link_as_bb($text);
    $text = formatting_as_bb($text);
    return $text;
}

?>
