<?
require_once("../inc/util.inc");

page_head("Allowed HTML tags");

echo "
The following HTML tags are allowed in profiles, messages,
signatures, etc.:
<ul>
<li> &lt;b> (bold)
<li> &lt;i> (italics)
<li> &lt;a> (hyperlink)
<li> &lt;p> (paragraph)
<li> &lt;br> (break)
<li> &lt;pre> (preformatted)
<li> &lt;img> (image; height cannot exceed 450 pixels)
</ul>
";

page_tail();
?>
