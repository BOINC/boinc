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

page_head("BBCode tags");
echo "<p>
BBCode tags let you format text in your profile and message-board postings.
It's similar to HTML, but simpler. The tags start with a [ (where you would
have used &lt; in HTML) and end with ] (where you would have used &gt; in
HTML).</p>";

start_table();
row1("Examples");
row2_plain("[b]Bold[/b]", "<b>Bold</b>");
row2_plain("[i]Italic[/i]", "<i>Italic</i>");
row2_plain("[u]Underline[/u]", "<u>Underline</u>");
row2_plain("[size=15]Big text[/size]", "<span style=\"font-size: 15px\">Big text</span>");
row2_plain("[color=red]Red text[/color]", "<font color=\"red\">Red text</font></li>");
row2_plain("[url=http://google.com/]Google[/url]", "<a href=\"http://google.com/\">Google</a>");
row2_plain("[quote]Quoted text[/quote]", "use for quoted blocks of text");
row2_plain("[img]http://example.com/pic.jpg[/img]", "use to display an image");
row2_plain("[code]Code snippet here[/code]", "use to display some code");
row2_plain("[pre]Pre-formatted text[/pre]", "use to display pre-formatted (usually monospaced) text");
row2_plain("[list]<br>* Item 1<br>*Item2<br>[/list]", "<ul><li>Item 1</li><li>Item 2</li></ul>");
row2_plain("[trac]#1[/trac] or [trac]ticket:1[/trac]",
    "use to link to Trac ticket on BOINC website: <a href=\"http://boinc.berkeley.edu/trac/ticket/1\">#1</a>");
row2_plain("[trac]wiki:WebForum[/trac]",
    "use to link to Trac Wiki on BOINC website: <a href=\"http://boinc.berkeley.edu/trac/wiki/WebForum\">WebForum</a>");
row2_plain("[trac]changeset:12345[/trac]",
    "use to link to SVN changeset on BOINC website: <a href=\"http://boinc.berkeley.edu/trac/changeset/12345\">[12345]</a>");
end_table();

echo "<p>
If you don't close a tag or don't specify a parameter correctly,
the raw tag itself will display instead of the formatted text.</p>
";
page_tail();
?>
