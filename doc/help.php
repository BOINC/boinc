<?php
require_once("docutil.php");
require_once("spoken_languages.php");
require_once("help_db.php");
require_once("../html/inc/translation.inc");

page_head("Live help via Internet phone or email");

echo "
<p>
".sprintf(tr(HELP_P1_1), "<ul><li>", "<li>", "<li>", "<li>")."
</ul>
<p>
".sprintf(tr(HELP_P1_2), "<a href=\"http://www.skype.com\">", "</a>", "<a href=\"http://www.skype.com\">", "</a>")."
<p>
".tr(HELP_P1_3),"

<p>
".tr(HELP_P1_4),"
<p>
";

$langs = get_languages();
sort($langs);
$n = 0;
foreach ($langs as $lang) {
    $lang_enc = urlencode($lang);
    if ($n) {
        echo "<br>";
    }
    $n++;
    echo "<a href=\"help_lang.php?lang=$lang_enc\"><b>$lang</b></a>";
}
echo "
<h2>".tr(HELP_HEADING3)."</h2>
<p>
".sprintf(tr(HELP_P3_1), "<a href=\"trac/wiki/HelpVolunteer\">", "</a>")."
<p>
".sprintf(tr(HELP_P3_2), "<a href=\"help_vol_edit.php?edit_login=1\">", "</a>")."
";
page_tail();
?>
