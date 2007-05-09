<?php
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
row2_plain("[quote]Quoted[/quoted]", "use for quoted blocks of text");
row2_plain("[img]http://example.com/pic.jpg[/img]", "use to display an image");
row2_plain("[code]Code snippet here[/code]", "use to display some code");
row2_plain("[pre]Pre-formatted text here[/pre]", "use to display pre-formatted (usually monospaced) text");
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
