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

require_once('../inc/sanitize_html.inc');

// Functions that process user-supplied text (e.g. messages)
// prior to displaying it to users.
// Goals:
// - Security (don't send evil javascript)
// - obey user preferences
// - improve formatting (e.g., convert newlines to <br> tags)

class output_options {
    var $bb2html;            // BBCode as HTML? (on)
    var $images_as_links;    // Images as hyperlinks? (off)
    var $link_popup;        // Links in new windows? (off)
    var $nl2br;                // Convert newlines to <br>'s? (on)
    var $htmlitems;            // Convert special chars to HTML entities? (on)
    var $htmlscrub;            // Scrub "bad" HTML tags? (off)
    var $highlight_terms;// Array of terms to be highlighted (off)    

    // Constructor - set the defaults.

    function output_options() {
        $this->bb2html = 1;
        $this->images_as_links = 0;
        $this->link_popup = 0;
        $this->nl2br = 1;
        $this->htmlitems = 1;
        $this->htmlscrub = 0;
        $this->highlight_terms = 0;
        return true;
    }

    // Define the terms to be highlighted (for use with searches and such)

    function setHighlightTerms($terms) {
        if (is_array($terms)) {
            $this->highlight_terms = $terms;
        } else {
            return false;
        }
        return true;
    }
}

// Do the actual transformation of the text.
// TODO: Make this part of the above class.

function output_transform($text, $options = NULL) {
    // Options is a output_options object, defined above
    if (!$options) {
        $options = new output_options; // Defaults in the class definition
    }
    if ($options->htmlitems) {
        //$text = htmlentities($text);
        $text = htmlspecialchars($text);
    }
    if (is_array($options->highlight_terms)) {
        $text = highlight_terms($text, $options->highlight_terms);
    }    
//    if ($options->htmlscrub) {
//        $text = sanitize_html($text);
//    }
    if ($options->nl2br) {
        $text = nl2br($text);
    }
    if ($options->bb2html) {
        $text = bb2html($text);
    }
    if ($options->images_as_links) {
        $text = image_as_link($text);
    }
    if ($options->link_popup) {
        $text = externalize_links($text);
    }
    return $text;
}

function get_output_options($user) {
    $options = new output_options();
    if ($user) {
        if ($user->prefs->images_as_links) $options->images_as_links = 1;
        if ($user->prefs->link_popup) $options->link_popup = 1;
    }
    return $options;
}

// Converts bbcode to proper HTML
// If $export is true, don't use BOINC CSS

function bb2html($text, $export=false) {
    $urlregex = "(?:\"?)(?:(http\:\/\/)?)([^\[\"<\ ]+)(?:\"?)";
    $httpsregex = "(?:\"?)https\:\/\/([^\[\"<\ ]+)(?:\"?)";
    // List of allowable tags
    $bbtags = array (
        "@\[pre\](.*?)\[/pre\]@eis",
        "@\[code\](.*?)\[/code\]@eis",
        "@\[b\](.*?)\[/b\]@is",
        "@\[i\](.*?)\[/i\]@is",
        "@\[u\](.*?)\[/u\]@is",
        "@\[sup\](.*?)\[/sup\]@is",
        "@\[url=$httpsregex\](.*?)\[/url\]@is",
        "@\[url\]$httpsregex\[/url\]@is",
        "@\[link=$urlregex\](.*?)\[/link\]@is",
        "@\[link\]$urlregex\[/link\]@is",
        "@\[url=$urlregex\](.*?)\[/url\]@is",
        "@\[url\]$urlregex\[/url\]@is",
        "@\[quote=(.*?)\](.*?)\[/quote\]@is",
        "@\[quote\](.*?)\[/quote\]@is",
        "@\[list\](.*?)\[/list\]@is",
        "@\[list=1\](.*?)\[/list\]@is",  
        "@\[img\]$urlregex\[/img\]@is",
        "@\[color=(?:\"?)(.{3,8})(?:\"?)\](.*?)\[/color\]@is",
        "@((?:<ol>|<ul>).*?)\n\*([^\n]+)\n(.*?(</ol>|</ul>))@is",
        "@\[size=([1-9]|[0-2][0-9])\](.*?)\[/size\]@is",
        "@\[mailto\](.*?)\[/mailto\]@is",
        "@\[email\](.*?)\[/email\]@is",
        "@\[trac\](?:\#|ticket:)(\d+)\[/trac\]@is",
        "@\[trac\]wiki:(.*?)\[/trac\]@is",
        "@\[trac\]changeset:(\d+)\[/trac\]@is"
        //Note:    The above list array member ensures we're within a list
        //when doing list item transformations.
        //TODO: Make sure we're not between two lists
    );
    
    // What the above tags are turned in to
    if ($export) {
        $htmltags = array (
            "'<pre>'.stop_recursion(remove_br('\\1')).'</pre>'",
            "'<code>'.stop_recursion('\\1').'</code>'",
            "<b>\\1</b>",
            "<i>\\1</i>",
            "<u>\\1</u>",
            "<sup>\\1</sup>",
            "<a href=\"https://\\1\" rel=\"nofollow\">\\2</a>",
            "<a href=\"https://\\1\" rel=\"nofollow\">https://\\1</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">\\3</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">http://\\2</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">\\3</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">http://\\2</a>",
            "<i>\\1 wrote:</i><blockquote>\\2</blockquote>",
            "<blockquote>\\1</blockquote>",
            "<ul>\\1</ul><p>",
            "<ol>\\1</ol><p>",
            "<img hspace=\"8\" src=\"http://\\2\"> ",
            "<font color=\"\\1\">\\2</font>",
            "\\1<li>\\2\n\\3",
            "<span style=\"font-size: \\1px;\">\\2</span>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"http://boinc.berkeley.edu/trac/ticket/\\1\">#\\1</a>",
            "<a href=\"http://boinc.berkeley.edu/trac/wiki/\\1\">\\1</a>",
            "<a href=\"http://boinc.berkeley.edu/trac/changeset/\\1\">[\\1]</a>"
        );
    } else {
        $htmltags = array (
            "'<div class=\"pre\">'.stop_recursion(remove_br('\\1')).'</div>'",
            "'<div class=\"code\">'.stop_recursion('\\1').'</div>'",
            "<b>\\1</b>",
            "<i>\\1</i>",
            "<u>\\1</u>",
            "<sup>\\1</sup>",
            "<a href=\"https://\\1\" rel=\"nofollow\">\\2</a>",
            "<a href=\"https://\\1\" rel=\"nofollow\">https://\\1</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">\\3</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">http://\\2</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">\\3</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">http://\\2</a>",
            "<div style='font-style: oblique'>\\1 wrote:</div><blockquote class='postbody'>\\2</blockquote>",
            "<blockquote class='postbody'>\\1</blockquote>",
            "<ul>\\1</ul><p>",
            "<ol>\\1</ol><p>",
            "<img hspace=\"8\" src=\"http://\\2\"> ",
            "<font color=\"\\1\">\\2</font>",
            "\\1<li>\\2\n\\3",
            "<span style=\"font-size: \\1px;\">\\2</span>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"http://boinc.berkeley.edu/trac/ticket/\\1\">#\\1</a>",
            "<a href=\"http://boinc.berkeley.edu/trac/wiki/\\1\">\\1</a>",
            "<a href=\"http://boinc.berkeley.edu/trac/changeset/\\1\">[\\1]</a>"
        );
    }
    
    // Do the actual replacing - iterations for nested items
    $lasttext = "";
    $i = 0;
    // $i<1000 to prevent DoS
    while ($text != $lasttext && $i<1000) {
        $lasttext = $text;
        $text = preg_replace($bbtags,$htmltags,$text);
        $i = $i + 1;
    }
    return $text;
}

// Removes any <br> tags added by nl2br which are not wanted,
// for example inside <pre> containers
// The original \n was retained after the br when it was added
//
function remove_br($text){
    return str_replace("<br />", "", $text);
}


// Stops recursion of BBCode by escaping any [ in the given text
// @param $text The text to stop recursion in
// @return A text that no longer can cause recursion
//
function stop_recursion($text){
    return str_replace("[", "&#91;", $text);
}

// Make links open in new windows.
function externalize_links($text) {
    // TODO:  Convert this to PCRE
    $i=0;$linkpos=true;
    while (true){                                //Find a link
        $linkpos=strpos($text,"<a ",$i);
        if ($linkpos===false) break;
        $out.= substr($text,$i,$linkpos-$i)."<a target=\"_new\" ";        //Replace with target='_new'
        $i=$linkpos+3;
    }
    $out.=substr($text,$i);
    return $out;
}

// Converts image tags to links to the images.

function image_as_link($text){
    /* This function depends on sanitized HTML */
    // Build some regex (should be a *lot* faster)
    $pattern = '@<img([\S\s]+?)src=([^>]+?)>@si';
    $replacement = '<a href=${2}>[Image link]</a>'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);
    return $text;
}

// Highlight terms in text (most likely used with searches)

function highlight_terms($text, $terms) {
    $search = $terms;
    $replace = array();

    foreach ($search as $key => $value) {
        $replace[$key] = "<span class=\"highlight\">".$value."</span>";
    }
    if (substr(phpversion(), 0, 1) > 4) {   // PHP 4.x doesn't support str_ireplace
        return str_ireplace($search, $replace, $text);
    } else {
        return str_replace($search, $replace, $text);
    }
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
