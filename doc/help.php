<?php
require_once("docutil.php");
require_once("spoken_languages.php");
require_once("help_db.php");
require_once("../html/inc/translation.inc");

page_head(tr(HELP_TITLE));

echo "
<h2>".tr(HELP_HEADING1)."</h2>
<p>
".sprintf(tr(HELP_P1_1), "<ul><li>", "<li>", "<li>", "<li>")."
</ul>
<p>
".sprintf(tr(HELP_P1_2), "<a href=http://www.skype.com>", "</a>", "<a href=http://www.skype.com>", "</a>")."
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
    if (!$n) {
    } else if (!($n%5)) {
        echo "<br>";
    } else {
        echo " | ";
    }
    $n++;
    echo "<a href=help_lang.php?lang=$lang_enc><b>$lang</b></a>";
}
echo "
<h2>".tr(HELP_HEADING2)."</h2>
<p>
<ul>
<li> <a href=trac/wiki/RunningBoinc>".tr(HELP_P2_ITEM1)."</a>
<li> <a href=trac/wiki/TroubleshootClient>".tr(HELP_P2_ITEM2)."</a>
<li> <a href=links.php>".tr(HELP_P2_ITEM3)."</a>
<li> <a href=dev/>".tr(HELP_P2_ITEM4)."</a>
<li> ".tr(HELP_P2_ITEM5)."
</ul>
<h2>".tr(HELP_HEADING3)."</h2>
<p>
".sprintf(tr(HELP_P3_1), "<a href=trac/wiki/HelpVolunteer>", "</a>")."
<p>
".sprintf(tr(HELP_P3_2), "<a href=help_vol_edit.php?edit_login=1>", "</a>")."
";
page_tail();
?>
