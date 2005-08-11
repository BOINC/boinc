<?php
require_once("../inc/util.inc");

page_head("BBCode tags");
echo "

<p>
BBCode tags let you format text in your profile and message-board postings.
It's similar to HTML, but simpler.
The tags start with a [ (where you would have used &lt; in HTML)
and end with ]
(where you would have used &gt; in HTML).</p>
<p>Examples:</p>
<ul>
	<li>[b]Bold[/b] to <b>Bold</b></li>
	<li>[i]Italic[/i] to <i>Italic</i></li>
	<li>[u]Underline[/u] to <u>Underline</u></li>
	<li>[size=15]Big text[/size] to <span style=\"font-size: 15px\">Big text</span></li>
	<li>[color=red]Red text[/color] to <font color=\"red\">Red text</font></li>
	<li>[url=http://google.com/]Google[/url] to <a href=\"http://google.com/\">Google</a></li>
	<li>[quote]Quoted[/quote] for quoted blocks of text</li>
	<li>[img]http://some.web.site/pic.jpg[/img] to display an image</li>
	<li>[code]Code snippet here[/code] to display some code</li>
	<li>[pre]Pre-formatted text here[/pre] to display some pre-formatted text</li>
</ul>
<p>Lists are also possible:<br/>[list]<br/>*Item 1<br/>*Item 2<br/>[/list] to:</p>
<ul>
	<li>Item 1</li>
	<li>Item 2</li>
</ul>
<p>
If you don't close a tag or don't specify a parameter correctly,
the raw tag itself will display instead of the formatted text.</p>
";
page_tail();
?>
