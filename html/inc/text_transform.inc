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

// Functions that process user-supplied text (e.g. messages)
// prior to displaying it to users.
// Goals:
// - Security (don't send evil javascript)
// - obey user preferences
// - improve formatting (e.g., convert newlines to <br> tags)

require_once('../inc/sanitize_html.inc');

class output_options {
    var $bb2html;            // BBCode as HTML? (on)
    var $images_as_links;    // Images as hyperlinks? (off)
    var $link_popup;        // Links in new windows? (off)
    var $nl2br;                // Convert newlines to <br>'s? (on)
    var $htmlitems;            // Convert special chars to HTML entities? (on)
    var $htmlscrub;            // Scrub "bad" HTML tags? (off)
    var $highlight_terms;// Array of terms to be highlighted (off)

    // Constructor - set the defaults.

    function __construct() {
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
        $text = htmlspecialchars($text, ENT_COMPAT | ENT_HTML401 | ENT_SUBSTITUTE);
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

// Converts bbcode to HTML
// If $export is true, don't use BOINC CSS

// handle [pre] and [code] separately because we need to remove <br />s
//
function replace_pre_code($text, $export) {
    if ($export) {
        $text = preg_replace_callback(
            "@\[pre\](.*?)\[/pre\]@is",
            function ($matches) {
                $x = remove_br(substr($matches[0], 5, -6));
                $x = htmlspecialchars($x, ENT_COMPAT, "UTF-8", false);
                $x = str_replace("[", "&#91;", $x);
                return "<pre>$x</pre>";
            },
            $text
        );
        return preg_replace_callback(
            "@\[code\](.*?)\[/code\]@is",
            function ($matches) {
                $x = remove_br(substr($matches[0], 6, -7));
                $x = htmlspecialchars($x, ENT_COMPAT, "UTF-8", false);
                $x = str_replace("[", "&#91;", $x);
                return "<code>$x</code>";
            },
            $text
        );
    } else {
        $text = preg_replace_callback(
            "@\[pre\](.*?)\[/pre\]@is",
            function ($matches) {
                $x = remove_br(substr($matches[0], 5, -6));
                $x = htmlspecialchars($x, ENT_COMPAT, "UTF-8", false);
                $x = str_replace("[", "&#91;", $x);
                return "<pre style=\"white-space:pre-wrap; \">$x</pre>";
            },
            $text
        );
        return preg_replace_callback(
            "@\[code\](.*?)\[/code\]@is",
            function ($matches) {
                $x = remove_br(substr($matches[0], 6, -7));
                $x = htmlspecialchars($x, ENT_COMPAT, "UTF-8", false);
                $x = str_replace("[", "&#91;", $x);
                return "<pre style=\"white-space:pre-wrap; \">$x</pre>";
            },
            $text
        );
    }
}

function bb2html($text, $export=false) {
    $urlregex = "(?:\"?)(?:(http\:\/\/)?)([^\[\"<\ ]+)(?:\"?)";
    // NOTE:
    // This matches https:// too; I don't understand why.
    // sample results:
    // Array
    // (
    //     [0] => [img]https://a.b.c[/img]
    //     [1] =>
    //     [2] => https://a.b.c
    // )
    // Array
    // (
    //     [0] => [img]http://a.b.c[/img]
    //     [1] => http://
    //     [2] => a.b.c
    // )

    $httpsregex = "(?:\"?)https\:\/\/([^\[\"<\ ]+)(?:\"?)";
    // List of allowable tags
    $bbtags = array (
        "@\[b\](.*?)\[/b\]@is",
        "@\[i\](.*?)\[/i\]@is",
        "@\[u\](.*?)\[/u\]@is",
        "@\[s\](.*?)\[/s\]@is",
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
        "@\[sm_img\]$urlregex\[/sm_img\]@is",
        "@\[color=(?:\"?)(.{3,8})(?:\"?)\](.*?)\[/color\]@is",
        "@((?:<ol>|<ul>).*?)\n\*([^\n]+)\n(.*?(</ol>|</ul>))@is",
        "@\[size=([1-9]|[0-2][0-9])\](.*?)\[/size\]@is",
        "@\[mailto\](.*?)\[/mailto\]@is",
        "@\[email\](.*?)\[/email\]@is",
        "@\[github\](?:\#|ticket:)(\d+)\[/github\]@is",
        "@\[github\]wiki:(.*?)\[/github\]@is",
    );

    // What the above tags are turned in to
    if ($export) {
        $htmltags = array (
            "<b>\\1</b>",
            "<i>\\1</i>",
            "<u>\\1</u>",
            "<s>\\1</s>",
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
            "<img hspace=\"8\" src=\"\\1\\2\"> ",
            "<img hspace=\"8\" width=400 src=\"\\1\\2\"> ",
            "<font color=\"\\1\">\\2</font>",
            "\\1<li>\\2\n\\3",
            "<span style=\"font-size: \\1px;\">\\2</span>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"https://github.com/BOINC/boinc/issues/\\1\">#\\1</a>",
            "<a href=\"https://github.com/BOINC/boinc/wiki/\\1\">\\1</a>",
        );
    } else {
        $htmltags = array (
            "<b>\\1</b>",
            "<i>\\1</i>",
            "<u>\\1</u>",
            "<s>\\1</s>",
            "<sup>\\1</sup>",
            "<a href=\"https://\\1\" rel=\"nofollow\">\\2</a>",
            "<a href=\"https://\\1\" rel=\"nofollow\">https://\\1</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">\\3</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">http://\\2</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">\\3</a>",
            "<a href=\"http://\\2\" rel=\"nofollow\">http://\\2</a>",
            "<em>\\1 wrote:</em><blockquote>\\2</blockquote>",
            "<blockquote>\\1</blockquote>",
            "<ul>\\1</ul><p>",
            "<ol>\\1</ol><p>",
            "<img hspace=\"8\" class=\"img-responsive\" src=\"\\1\\2\"> ",
            "<img hspace=\"8\" width=400 src=\"\\1\\2\"> ",
            "<font color=\"\\1\">\\2</font>",
            "\\1<li>\\2\n\\3",
            "<span style=\"font-size: \\1px;\">\\2</span>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"mailto:\\1\">\\1</a>",
            "<a href=\"https://github.com/BOINC/boinc/issues/\\1\">#\\1</a>",
            "<a href=\"https://github.com/BOINC/boinc/wiki/\\1\">\\1</a>",
        );
    }

    // Do the actual replacing - iterations for nested items
    $lasttext = "";
    $i = 0;
    // $i<1000 to prevent DoS
    while ($text != $lasttext && $i<1000) {
        $lasttext = $text;
        $text = replace_pre_code($text, $export);
        $text = preg_replace($bbtags, $htmltags, $text);
        $i = $i + 1;
    }
    $text = str_replace("<ul>", '<ul style="word-break:break-word;">', $text);
    $text = str_replace("<ol>", '<ol style="word-break:break-word;">', $text);
    return $text;
}

// Removes any <br> tags added by nl2br which are not wanted,
// for example inside <pre> containers
// The original \n was retained after the br when it was added
//
function remove_br($text){
    return str_replace("<br />", "", $text);
}

// Make links open in new windows.
//
function externalize_links($text) {
    // TODO:  Convert this to PCRE
    $i=0;
    $linkpos=true;
    $out = "";
    while (true){
        //Find a link
        //
        $linkpos=strpos($text, "<a ", $i);
        if ($linkpos===false) break;

        //Replace with target='_new'
        //
        $out .= substr($text, $i, $linkpos-$i)."<a target=\"_new\" ";
        $i = $linkpos+3;
    }
    $out .= substr($text, $i);
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
        $replace[$key] = "<span class=\"mark\">".$value."</span>";
    }
    if (substr(phpversion(), 0, 1) > 4) {   // PHP 4.x doesn't support str_ireplace
        return str_ireplace($search, $replace, $text);
    } else {
        return str_replace($search, $replace, $text);
    }
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
