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
require_once("../inc/language_names.inc");

$languages = get_supported_languages();
if (!is_array($languages)) {
    error_page("Language selection not enabled.  Project admins must run the update_translations.php script.");
}

$prefs = "";
if (isset($_SERVER["HTTP_ACCEPT_LANGUAGE"])) {
    $prefs = $_SERVER["HTTP_ACCEPT_LANGUAGE"];
    $prefs = sanitize_tags($prefs);
}

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

page_head(tra("Language selection"));

if (count($languages_in_use)) {
    $lang_code = $languages_in_use[0];
} else {
    $lang_code = 'en';
}
$cur_lang_desc = language_desc($lang_code);

echo "<p>",
    tra(
        "This web site is available in several languages. The currently selected language is %1.",
        $cur_lang_desc
    ),
    "</p><p>",
    tra(
        "Normally the choice of language is determined by your browser's language setting, which is: %1.  You can change this setting using:",
        "<b>$prefs</b>"
    ),
    "</p><ul>",
    "<li>",
    tra("Firefox: Tools/Options/General"),
    "<li>",
    tra("Microsoft IE: Tools/Internet Options/Languages"),
    "</ul>",
    "<p>",
    tra(
        "Or you can select a language from the following menu:"
    ),
    "</p>"
;

echo '<div class="col-sm-3">
';
language_form($lang_code);
echo '</div>
';

echo "<br clear=all><p> </p>",
    tra("Translations are done by volunteers.  If your native language is missing or incomplete, %1 you can help translate %2.",
    '<a href="https://github.com/BOINC/boinc/wiki/TranslateIntro">', '</a>'),
    "</p>"
;
page_tail();
?>
