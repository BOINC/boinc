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
    var $closeTags;            // Close extra HTML tags? (on)
    var $nl2br;                // Convert newlines to <br>'s? (on)
    var $htmlitems;            // Convert special chars to HTML entities? (on)
    var $htmlscrub;            // Scrub "bad" HTML tags? (off)
    var $highlight_terms;// Array of terms to be highlighted (off)    

    // Constructor - set the defaults.

    function output_options() {
        $this->bb2html = 1;
        $this->images_as_links = 0;
        $this->link_popup = 0;
        $this->closeTags = 1;
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
    if (is_array($options->highlight_terms)) {
        $text = highlight_terms($text, $options->highlight_terms);
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

function bb2html($text) {
    $urlregex = "(?:\"?)(?:(http\:\/\/)?)([^\[\"<\ ]+)(?:\"?)";
    $httpsregex = "(?:\"?)https\:\/\/([^\[\"<\ ]+)(?:\"?)";
    // List of allowable tags
    $bbtags = array (
        "@\[b\](.*?)\[/b\]@is",
        "@\[i\](.*?)\[/i\]@is",
        "@\[u\](.*?)\[/u\]@is",
        "@\[url=$httpsregex\](.*?)\[/url\]@i",
        "@\[url\]$httpsregex\[/url\]@i",
        "@\[link=$urlregex\](.*?)\[/link\]@i",
        "@\[link\]$urlregex\[/link\]@i",
        "@\[url=$urlregex\](.*?)\[/url\]@i",
        "@\[url\]$urlregex\[/url\]@i",
        "@\[quote=(.*?)\](.*?)\[/quote\]@is",
        "@\[quote\](.*?)\[/quote\]@is",
        "@\[list\](.*?)\[/list\]@is",
        "@\[list=1\](.*?)\[/list\]@is",  
        "@\[pre\](.*?)\[/pre\]@is",
        "@\[img\]$urlregex\[/img\]@is",
        "@\[color=(?:\"?)(.{3,8})(?:\"?)\](.*?)\[/color\]@is",
        "@((?:<ol>|<ul>).*?)\n\*([^\n]+)\n(.*?(</ol>|</ul>))@is",
        "@\[size=([1-9]|[0-2][0-9])\](.*?)\[/size\]@is",
        "@\[code\](.*?)\[/code\]@is",
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
    $htmltags = array (
        "<b>\\1</b>",
        "<i>\\1</i>",
        "<u>\\1</u>",
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
        "<pre>\\1</pre>",
        "<img src=\"http://\\2\">",
        "<font color=\"\\1\">\\2</font>",
        "\\1<li>\\2</li>\n\\3",
        "<span style=\"font-size: \\1px;\">\\2</span>",
        "<div class=\"code\">\\1</div>",
        "<a href=\"mailto:\\1\">\\1</a>",
        "<a href=\"mailto:\\1\">\\1</a>",
        "<a href=\"http://boinc.berkeley.edu/trac/ticket/\\1\">#\\1</a>",
        "<a href=\"http://boinc.berkeley.edu/trac/wiki/\\1\">\\1</a>",
        "<a href=\"http://boinc.berkeley.edu/trac/changeset/\\1\">[\\1]</a>"
    );
    
    // Do the actual replacing - iterations for nested items
    $lasttext = "";
    $i = 0;
    // $i<20 to prevent DoS
    while ($text != $lasttext && $i<20) {
        $lasttext = $text;
        $text = preg_replace($bbtags,$htmltags,$text);
        $i = $i + 1;
    }
    return $text;
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
    $pattern = '@<img src=([^>]+)>@si'; // Gives us the URL in ${1}...
    $replacement = '<a href=${1}>[Image Link]</a>'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);
    return $text;
}

// Closes open HTML tags.  Not quite perfect...

function closeTags($str = null) {
    // Function from http://episteme.arstechnica.com/eve/ubb.x/a/tpc/f/6330927813/m/139006252731/r/287008552731#287008552731
    // (thanks Ageless for finding it)
    // Edited by Rob to better fit in with boinc's needs
    
    // List of tags to check $str for
    // TODO:  Adapt to use the pre-existing array of tags above
    $tags = array('b', 'i', 'a', 'p', 'font[^>]?', 'strong', 'ul', 'li', 'pre', 'blockquote', 'u');
    // Note on $tags - no br or img, as they have no closing tags - can we define this above?
    // Maybe define two arrays, those with closing tags and those without, and combine the
    // two of them for the standard HTML sanitizing function?

    // Don't do anything if the string is too short
    if (strlen($str) < 3) {
        return $str;
    } else {
        // Loop over $str and count the opening and closing for each tag in $tags
        foreach ($tags as $tag) {
            $m = array();
            $o = preg_match_all("/<(".$tag.")>/", $str, $m);
            $c = substr_count($str, "</{$tag}>");

            $open[$tag]  = ($o < $c) ? $c - $o : 0;
            $close[$tag] = ($c < $o) ? $o - $c : 0;

            // Debuggin'
            //echo "<pre>Tag: {$tag}\nOpen: {$o}\nClose: {$c}\nOT: {$open[$tag]}\nCT: {$close[$tag]}</pre><hr />";
        }

        // Prepend the return string with an opening tag as needed
        /* $pre = '';  ...uhh... doesn't work right

        foreach ($open as $tag => $cnt) {
            $pre .= ($cnt > 0) ? "<{$tag}>" : '';
        }  */

        // Append the return string with a closing tag as needed
        $post = '';

        foreach ($close as $tag => $cnt) {
            $post .= ($cnt > 0) ? "</{$tag}>" : '';
        }

        return /*$pre.*/$str.$post;
    }
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
