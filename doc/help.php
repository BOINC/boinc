<?php
require_once("docutil.php");
require_once("spoken_languages.php");
require_once("help_db.php");

page_head("BOINC Online Help");

echo "
<h2>Need help?</h2>
<p>
BOINC Online Help lets you talk one-on-one
with experienced BOINC users, who can:
<ul>
<li> answer questions about BOINC and volunteer computing;
<li> walk you through the process of installing and using BOINC;
<li> troubleshoot any problems you might have.
</ul>
<p>
BOINC Online Help is based on
<a href=http://www.skype.com>Skype</a>,
an Internet-based telephone system.
Skype is free (both the software and the calls).
If you don't already have Skype, please
<a href=http://www.skype.com>download and install it now</a>.
When you're finished, return to this page.
<p>
The best way to get help is by voice,
for which you need either built-in microphone and speakers
or an external headset for your computer.
Alternatively, you can use Skype's text-based chat system
to communicate with Help Volunteers.

<p>
Volunteers speaking several languages are available.
Please select a language:
<p>
";

$langs = get_languages();
$first = true;
foreach ($langs as $lang) {
    $lang_enc = urlencode($lang);
    if ($first) {
        $first = false;
    } else {
        echo " | ";
    }
    echo "<a href=help_lang.php?lang=$lang_enc><b>$lang</b></a>";
}
echo "
<h2>Other sources of help</h2>
<p>
You can also get information and advice from:
<ul>
<li> <a href=http://boinc.berkeley.edu/>The BOINC web site</a>.
<li> <a href=http://boinc.berkeley.edu/links.php>BOINC-related web sites</a>.
<li> <a href=http://boinc.berkeley.edu/dev/>The BOINC message boards</a>.
<li> The message boards on any BOINC-based project.
</ul>
<h2>Be a Help Volunteer</h2>
<p>
If you are an experienced BOINC user, we encourage you to
<a href=help_volunteer.php>become a Help Volunteer</a>.
It's a great way to help the cause of scientific research
and volunteer computing -
and it's fun!
<p>
If you're already a Help Volunteer: to edit your settings,
<a href=help_vol_edit.php?edit_login=1>click here</a>.
";
page_tail();
?>
