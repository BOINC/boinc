<?php
require_once("../inc/util.inc");

page_head("Allowed HTML tags");

echo "
The following HTML tags are allowed in profiles, messages,
signatures, etc.:
<ul>
<li> &lt;b> or &lt;strong> (bold)
<li> &lt;i> or &lt;em> (italics)
<li> &lt;a> (hyperlink)
<li> &lt;p> (paragraph)
<li> &lt;br> (break)
<li> &lt;pre> (preformatted)
<li> &lt;img> (image; height cannot exceed 450 pixels.
Please do not link to images without
permission of the web site where the image is hosted.)
</ul>
You can also use ampersand notation for special characters.
";

page_tail();
?>
