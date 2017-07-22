<?php
require_once("docutil.php");
require_once("spoken_languages.php");
require_once("help_db.php");
require_once("../html/inc/translation.inc");

page_head(tra("Live help via Skype or email"));

echo "
<p>
".tra("BOINC Live Help lets you get one-on-one help from an experienced BOINC user who can answer questions about BOINC, help you install BOINC, and troubleshoot problems.")."
</ul>
<p>
".tra("You can communicate with a helper")."
<ul>
<li> ".tra("by email")."
<li>
".tra("
by voice, using %1 Skype %2, a free Internet-based telephone system. If you don't already have Skype, you can %3 download and install it now %4.  When you're finished, return to this page.",
    "<a href=\"https://www.skype.com\">",
    "</a>",
    "<a href=\"https://www.skype.com\">",
    "</a>"
)."
<li> ".tra("using Skype chat")."
</ul>
<p>
".tra("Volunteers speaking many languages are available. Please select a language (number of helpers is shown):"),"
<p>
";

$langs = get_languages2();
$i = 0;
foreach ($langs as $lang=>$n) {
    $lang_enc = urlencode($lang);
    if ($i) {
        echo " &middot; ";
    }
    $i++;
    echo "<a href=\"help_lang.php?lang=$lang_enc\"><b>$lang</b></a> ($n)";
}
echo "
<h2>".tra("Be a Help Volunteer")."</h2>
<p>
".sprintf(
    tra("If you're an experienced BOINC user, we encourage you to %sbecome a Help Volunteer%s.  It's a great way to help the cause of scientific research and volunteer computing - and it's fun!"),
    "<a href=http://boinc.berkeley.edu/wiki/Help_volunteer>",
    "</a>"
)."
<p>
".sprintf(
    tra("If you're already a Help Volunteer: to edit your settings, %sclick here%s."),
    "<a href=\"help_vol_edit.php?edit_login=1\">",
    "</a>"
)."
";
page_tail();
?>
