<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


require_once("../inc/util.inc");
require_once("../inc/translation.inc");

$languages = getSupportedLanguages();
if (!is_array($languages)) {
    error_page("Language selection not enabled.  Project admins must run the update_translations.php script.");
}

$prefs = $_SERVER["HTTP_ACCEPT_LANGUAGE"];

$set_lang = get_str("set_lang", true);
if ($set_lang){
    if (!in_array($set_lang, $languages) && $set_lang!="auto"){
        error_page("Language not supported");
    } else {
        send_cookie('lang', $set_lang, true);
        header("Location: index.php");
        exit;
    }
}

page_head("Language selection");

echo "
    <p>
    This web site is available in several languages.
    The currently selected language is: <em>".tra("LANG_NAME_INTERNATIONAL")."</em> (".tra("LANG_NAME_NATIVE").").
    <p>
    Normally the choice of language
    is determined by your browser's language setting,
    which is: <b>$prefs</b>.
    You can change this setting using:
    <ul>
    <li>Firefox: Tools/Options/General
    <li>Microsoft IE: Tools/Internet Options/Languages
    </ul>
    <p>
    Or you can select a language by clicking on one of the links.
    This will send your browser a cookie;
    make sure your browser accepts cookies from our domain.
    </p>
";

start_table();
row2("Language symbol", "Language name (click to select)");
row2("",
    "<a href=language_select.php?set_lang=auto>Use browser language setting</a>"
);
sort($languages);
foreach ($languages as $language) {
    row2(
        "<a href=\"language_select.php?set_lang=$language\">$language</a>",
        "<a href=\"language_select.php?set_lang=$language\">".tr_specific("LANG_NAME_INTERNATIONAL", $language)." (".tr_specific("LANG_NAME_NATIVE", $language).")</a>"
    );
}
end_table();
echo "
    <p>
    Translations are done by volunteers.
    If your native language is not here,
    <a href=translate.php>you can help</a>.
";
page_tail();
?>
