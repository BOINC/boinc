<?php
require_once("docutil.php");
page_head("Language customization of the work manager");
echo "
    Menu names and other text in the work manager are stored in
    a file called <i>BOINC Manager.po</i>.
    The release uses American English.
    Many other languages are available.
    The BOINC distribution includes all current language files.

	A tool you can use to modify the 'po' file is called poEdit
	and can be found here:<br>
	<a src=http://sourceforge.net/projects/poedit/>http://sourceforge.net/projects/poedit/</a>
	
    <p>
    To submit a new language file,
    please email it to Rom Walton (rwalton at ssl.berkeley.edu).
";
page_tail();
?>
