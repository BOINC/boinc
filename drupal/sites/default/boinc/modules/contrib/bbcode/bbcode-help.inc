<?php
// $Id: bbcode-help.inc,v 1.8 2007/01/28 21:18:45 naudefj Exp $

function _bbcode_filter_tip() {
  return t("<a id=\"filter-bbcode\"></a><h2>BBCode Guide</h2>

<p>
 BBCode allows you to specify formatting rules for your text,
 even if you are not allowed to use HTML in your posts. BBCode
 originated from the forum software named PHPBB, and this site
 has a special implementation of it.
</p>
<p>
 In BBCode terms, you use \"tags\" to add formatting to your text. Every
 tag is enclosed in <strong>[</strong> and <strong>]</strong> brackets. 
 If you want to mark some region in your text, you need to use an opening 
 tag and a closing tag. Closing tags start with <strong>[/</strong>, as
 you will see in the examples below. If you mistype a tag or forget to 
 close it, you will not get the desired formatting.
</p>
<p>
When using the Rich-text editor, loaded by default, ENTER will place a
new paragraph and SHIFT-ENTER will place a new line. There are some
BBCode tags that will not work with ENTER (new paragraph), e.g., the
[list] tag. In general, when entering a line break within a BBCode
tag, please use SHIFT-ENTER.
</p>

<h3>Simple text formatting</h3>

<p>
 BBCode allows you to make some parts of your texts stand out from the
 context by adding <strong>[b]</strong>old, <strong>[i]</strong>talic,
 <strong>[u]</strong>nderlined and <strong>[s]</strong>trikeout formatting 
 to them. The <strong>[color]</strong>, <strong>[size]</strong> and 
 <strong>[font]</strong> tags allow you to change the color, size and font 
 of portions of the text you enclose with these tags. Both require a 
 parameter (which colour, how big, what font) that is suffixed to the name 
 of the tag by an equals sign (example below).  You should not repeat the 
 parameter in the closing tag!
</p>

<p>
 You can specify any recognized color name (red, blue, green, white, etc.)
 or a hexadecimal color value (#CDCDCD, #FFFFFF, etc.) as the parameter of
 a <strong>[color]</strong> tag. The <strong>[size]</strong> tag allows you
 to set the font size between 6 and 48, 6 being the smallest size.
 Note that using very large text is considered by many to be annoying, and
 it is seldom a good idea to try to attract more attention to your post in
 this way. The <strong>[font]</strong> tag can be set to any valid font face,
 such as Arial, Arial Black, Courier, Courier New, Helvetica, Impact, 
 Times New Roman, Verdana, etc.
</p>

<table width=\"98%\">
 <tr>
  <th>usage</th><th>display</th>
 </tr>
 <tr>
  <td>I [b]need to do[/b] this by the weekend</td>
  <td>I <strong>need to do</strong> this by the weekend</td>
 </tr>
 <tr>
  <td>John said that [i]we should[/i] ask her</td>
  <td>John said that <em>we should</em> ask her</td>
 </tr>
 <tr>
  <td>I [u]would not like to[/u] offend you</td>
  <td>I <span style=\"text-decoration: underline;\">would not like to</span> offend you</td>
 </tr>
 <tr>
  <td>Let's correct this [s]mispelled[/s] misspelled word</td>
  <td>Let's correct this <s>mispelled</s> misspelled word</td>
 </tr>
 <tr>
  <td>Jane was at [color=blue]the coast[/color]</td>
  <td>Jane was at <span style=\"color: blue;\">the coast</span></td>
 </tr>
 <tr>
  <td>Joe was in [color=#FF0000]the forest[/color]</td>
  <td>Joe was in <span style=\"color: #FF0000;\">the forest</span></td>
 </tr>
 <tr>
  <td>You said: [size=30]HEY![/size]</td>
  <td>You said: <span style=\"font-size: 30px;\">HEY!</span></td>
 </tr>
 <tr>
  <td>She said: [font=Courier]What?[/font]</td>
  <td>She said: <span style=\"font-family: Courier;\">What?</span></td>
 </tr>
</table>

<h3>Creating links</h3>

<p>
 You have multiple options to specify links to other destinations in
 your posts.
</p>

<p>
 URLs (Uniform Resource Locators) starting with \"www\" or \"ftp\" (eg.
 www.example.com) are automatically recognized and replaced with links. 
 You can also use the <strong>[url]</strong> tag with a parameter to 
 specify a link with meaningful text to click on. If you use the url 
 tag without the parameter, the enclosed text is assumed to be a URL,
 and a link is created to that destination.
</p>
<p>
 Email addresses in posts are also automatically converted to email links. 
 For compatibility with common BBCode implementations, an 
 <strong>[email]</strong> tag is provided. 
</p>

<table width=\"98%\">
 <tr>
  <th>usage</th><th>display</th>
 </tr>
 <tr>
  <td>For more examples, visit www.example.com</td>
  <td>For more examples, visit <a href=\"http://www.example.com\" target=\"_blank\">www.example.com</a></td>
 </tr>
 <tr>
  <td>For more examples, visit http://example.com</td>
  <td>For more examples, visit <a href=\"http://example.com\" target=\"_blank\">http://example.com</a></td>
 </tr>
 <tr>
  <td>If you have questions ask me at joe@example.com</td>
  <td>If you have questions ask me at <a href=\"mailto:joe@example.com\">joe@example.com</a></td>
 </tr>
 <tr>
  <td>If you have questions ask me at [email]joe@example.com[/email]</td>
  <td>If you have questions ask me at <a href=\"mailto:joe@example.com\">joe@example.com</a></td>
 </tr>
 <tr>
  <td>We use [url=http://example.com/]the example site[/url] in these examples</td>
  <td>We use <a href=\"http://example.com/\" target=\"_blank\">the example site</a> in these examples</td>
 </tr>
 <tr>
  <td>We use [url]http://example.com/[/url] in these examples</td>
  <td>We use <a href=\"http://example.com/\" target=\"_blank\">http://example.com/</a> in these examples</td>
 </tr>
</table>

<h3>Displaying images</h3>

<p>
 The <strong>[img]</strong> tag allows you to display an image in your
 post. You need to specify a URL to the image, so it needs to be
 accessible somewhere on the internet. Beware of adding very large
 images to your text, or the page will load very slowly!
</p>

<p>
 If you enclose a URL in an <strong>[img]</strong> tag, then it will
 be replaced with code to display the image. For example <code>A good
 screenshot: [img]http://example.com/screenshot.png[/img]</code>
 will show you the screenshot (if it exists). 
</p>
<p>
 You can also specify the desired display dimensions of the image by
 adding a dimension parameter to the <strong>[img]</strong> tag. <code>A
 good screenshot: [img=640x480]http://example.com/screenshot.png[/img]</code>
 will display the image in 640x480 (though the full image will be
 downloaded). Do not use this to show a thumbnail of an image!
</p>

<p>
 You are free to link an image to an external destination by enclosing
 the <strong>[img]</strong> tag with a <strong>[url]</strong> tag: <code>See
 [url=http://example.com][img]http://example.com/screenshot.png[/img][/url]</code>.
</p>

<h3>Ordered and unordered lists</h3>

<p>
 The simplest list type is the unordered list, which means that there is
 no numbering applied to the elements. You can make such a list by enclosing
 the list elements in <strong>[list]</strong> opening and closing tags.
 Specify the start of one element with the <strong>[*]</strong> list element
 marker, which has no closing tag pair.
</p>

<p>
 To create an ordered list, you should add a parameter to the 
 <strong>[list]</strong> list tag specifying what type of ordered list 
 you would like to see. The possible parameters are \"i\", \"I\", \"1\", 
 \"a\", \"A\", \"c\", \"d\" and \"s\" which all correspond to the display 
 of the first list element.
</p>

<table width=\"98%\">
 <tr>
  <th>usage</th><th>display</th>
 </tr>
 <tr>
  <td><pre>
I love
 [list]
  [*]Oranges
  [*]Apples
  [*]Bananas
 [/list]
</pre></td>
  <td>
   I love 
   <ul>
    <li>Oranges</li>
    <li>Apples</li>
    <li>Bananas</li>
   </ul>
  </td>
 </tr>
 <tr>
  <td><pre>
I love
 [list=I]
  [*]Oranges
  [*]Apples
  [*]Bananas
 [/list]
</pre></td>
  <td>
   I love 
   <ol style=\"list-style-type: upper-roman;\">
    <li>Oranges</li>
    <li>Apples</li>
    <li>Bananas</li>
   </ol>
  </td>
 </tr>
 <tr>
  <td><pre>
I love
 [list=1]
  [*]Oranges
  [*]Apples
  [*]Bananas
 [/list]
</pre></td>
  <td>
   I love 
   <ol style=\"list-style-type: decimal;\">
    <li>Oranges</li>
    <li>Apples</li>
    <li>Bananas</li>
   </ol>
  </td>
 </tr>
</table>

<h3>Fixed-width text and block formatting</h3>

<p>
 You can use the <strong>[code]</strong> tag to add an inline fixed-width
 formatted part or to add a block of (usually program) code. If there is
 any newline present between the opening and closing tags, then a block
 will be displayed.
</p>

<table width=\"98%\">
 <tr>
  <th>usage</th><th>display</th>
 </tr>
 <tr>
  <td>Edit your [code]robots.txt[/code] file</td>
  <td>Edit your <code>robots.txt</code> file</td>
 </tr>
 <tr>
 </tr>
 <tr>
  <td>
An HTML title example:<br />
[code]<br />
&lt;head&gt;<br />
&nbsp;&lt;title&gt;Page Title&lt;/title&gt;<br />
&lt;/head&gt;<br />
[/code]
  </td>
  <td>
An HTML title example:<br />
<div class=\"codeblock\"><code>
&lt;head&gt;<br />
&nbsp;&lt;title&gt;Page Title&lt;/title&gt;<br />
&lt;/head&gt;<br />
</code>
  </td>
 </tr>
 <tr>
 </tr>
</table>

<h3>Text and block alignment</h3>

<p>
 You can also set the alignment of the text by using <strong>[left]</strong>,
 <strong>[right]</strong> and <strong>[center]</strong> tags. The 
 <strong>[float]</strong> tag can be used to place floating boxes in the text 
 (especially handy for images). You can specify the direction of the floating with
 <strong>[float=left]</strong> and <strong>[float=right]</strong>. The 
 <strong>[justify]</strong> tag can be used justify text on both sides of 
 the page.
</p>

<h3>Other supported tags</h3>

<p>
 It is possible to quote something that has already been posted, by just 
 putting <strong>[quote][/quote]</strong> tags around it. To quote a 
 specific person, use something like [quote=John]. Quote tags can 
 be nested.
</p>

<p>
  The <strong>[sub]</strong> and <strong>[sup]</strong> tags can be used 
  to add subscript and superscript text. For example,
  H[sub]2[/sub]O gives H<sub>2</sub>O, while 
  X[sup]3[/sup] gives X<sup>3</sup>.
</p>

<p>
 The <strong>[acronym]</strong> tag allow you to identify text as 
 an acronym and provide a description when users move their mouse 
 over the tag. For example,
 [acronym=Structured Query Language]SQL[/acronym] produces 
 <acronym title=\"Structured Query Language\">SQL</acronym>.
</p>

<p>
 The <strong>[abbr]</strong> tag allow you to identify text as 
 an abbreviation and provide a description when users move their mouse 
 over the tag. For example,
 [abbr=World Wide Web]WWW[/abbr] produces 
 <abbr title=\"World Wide Web\">WWW</abbr>.
</p>

<p>
 The <strong>[notag]</strong> tags prevent text inside the tags from being 
 parsed. This allows you to give examples of BBcode and not have it 
 converted to HTML. For example: [notag]These [b]tags[/b] are not 
 rendered[/notag] will produce \"These [b]tags[/b] are not rendered\".
</p>

<p>
 The <strong>[hr]</strong> tag draws a horizontal line across the page. Handy
 for separating chunks of text.
</p>

<h3>Using multiple formatting tags</h3>

<p>
 You can apply more than one formatting specification to a portion of some
 text. <code>I was at [b][i]the coast[/i][/b]</code> will be
 rendered as <code>I was at <strong><em>the coast</em></strong></code>.
</p>

<p>
 Make sure that you take care of the proper order of the opening and closing
 tags. You should close the tags in the opposite order in which you opened
 them. Otherwise you might get very strange rendering results. Also check
 your post with the preview function before submitting it, in case there are
 formatting errors due to improper BBCode usage.
</p>");

}
