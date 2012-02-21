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

require_once("../inc/util.inc");

check_get_args(array());

page_head(tra("BBCode tags"));
echo "<p>
".tra("BBCode tags let you format text in your profile and message-board postings.
It's similar to HTML, but simpler. The tags start with a [ (where you would
have used %1 in HTML) and end with ] (where you would have used %2 in
HTML).", "&lt;", "&gt;")."</p>";

start_table();
row1(tra("Examples"));
row2_plain("[b]".tra("Bold")."[/b]", "<b>".tra("Bold")."</b>");
row2_plain("[i]".tra("Italic")."[/i]", "<i>".tra("Italic")."</i>");
row2_plain("[u]".tra("Underline")."[/u]", "<u>".tra("Underline")."</u>");
row2_plain("[sup]".tra("Superscript")."[/sup]", "X<sup>".tra("Superscript")."</sup>");
row2_plain("[size=15]".tra("Big text")."[/size]", "<span style=\"font-size: 15px\">".tra("Big text")."</span>");
row2_plain("[color=red]".tra("Red text")."[/color]", "<font color=\"red\">".tra("Red text")."</font></li>");
row2_plain("[url=http://google.com/]".tra("link to website")."[/url]", "<a href=\"http://google.com/\">".tra("link to website")."</a>");
row2_plain("[quote]".tra("Quoted text")."[/quote]", tra("use for quoted blocks of text"));
row2_plain("[img]http://example.com/pic.jpg[/img]", tra("use to display an image"));
row2_plain("[code]".tra("Code snippet here")."[/code]", tra("use to display some code"));
row2_plain("[pre]".tra("Pre-formatted text")."[/pre]", tra("use to display pre-formatted (usually monospaced) text"));
row2_plain("[list]<br>* ".tra("Item 1")."<br>* ".tra("Item2")."<br>[/list]", "<ul><li>".tra("Item 1")."</li><li>".tra("Item 2")."</li></ul>");
row2_plain("[trac]#1[/trac] or [trac]ticket:1[/trac]",
    tra("use to link to Trac ticket on BOINC website").": <a href=\"http://boinc.berkeley.edu/trac/ticket/1\">#1</a>");
row2_plain("[trac]wiki:WebForum[/trac]",
    tra("use to link to Trac Wiki on BOINC website").": <a href=\"http://boinc.berkeley.edu/trac/wiki/WebForum\">WebForum</a>");
row2_plain("[trac]changeset:12345[/trac]",
    tra("use to link to SVN changeset on BOINC website").": <a href=\"http://boinc.berkeley.edu/trac/changeset/12345\">[12345]</a>");
end_table();

echo "<p>
".tra("If you don't close a tag or don't specify a parameter correctly,
the raw tag itself will display instead of the formatted text.")."</p>
";
page_tail();
?>
