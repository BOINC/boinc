<?php
require_once("docutil.php");
require_once("spoken_languages.php");
require_once("help_db.php");
require_once("../html/inc/translation.inc");

page_head("Live help via Internet phone or email");

echo "
<p>
".sprintf(
    tra("BOINC Online Help lets you talk one-on-one with experienced BOINC users, who can: %s answer questions about BOINC and volunteer computing; %s walk you through the process of installing and using BOINC; %s troubleshoot any problems you might have."),
    "<ul><li>",
    "<li>",
    "<li>",
    "<li>"
)."
</ul>
<p>
".sprintf(
    tra("BOINC Online Help is based on %sSkype%s, an Internet-based telephone system. Skype is free (both the software and the calls).  If you don't already have Skype, please %sdownload and install it now%s.  When you're finished, return to this page."),
    "<a href=\"http://www.skype.com\">",
    "</a>",
    "<a href=\"http://www.skype.com\">",
    "</a>"
)."
<p>
".tra("The best way to get help is by voice, for which you need either built-in microphone and speakers or an external headset for your computer.  You can also use Skype's text-based chat system or regular email (if you don't have Skype) to communicate with Help Volunteers."),"

<p>
".tra("Volunteers speaking several languages are available. Please select a language:"),"
<p>
";

$langs = get_languages();
sort($langs);
$n = 0;
foreach ($langs as $lang) {
    $lang_enc = urlencode($lang);
    if ($n) {
        echo " &middot; ";
    }
    $n++;
    echo "<a href=\"help_lang.php?lang=$lang_enc\"><b>$lang</b></a>";
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
