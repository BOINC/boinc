<?php
require_once("docutil.php");
page_head("Language customization of the work manager");
echo "
Menu names and other text in the work manager are stored in
a file called <i>BOINC Manager.po</i>.
The release uses American English.
Many other languages are available.
The BOINC distribution includes all current language files.

<p>
If you're bilingual,
you can help by making a translation for your non-English language.
<br>
Notes:
<ul>
<li>
You can use a tool called <a href=http://www.poedit.org>poEdit</a>
to modify the 'po' file.
	
<li>
You might want to subscribe to the
<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_loc>boinc_loc at ssl.berkeley.edu</a>
email list, which is devoted to discussion of BOINC-related translation.
<li>
To submit a new language file,
please email it to boinc_loc (boinc_loc at ssl.berkeley.edu).
</ul>
";
page_tail();
?>
