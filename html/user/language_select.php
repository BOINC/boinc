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

page_head(tra("Language selection"));

function language_name($code) {
    if ($code == 'en') {
        return "<em>en</em> (English)";
    } else {
        $lname = tr_specific("LANG_NAME_NATIVE", $code);
        if ($lname == 'English' || $lname == '') {
            return "<b>$code</b>";
        } else {
            return "<b>$code</b> ("
                .tr_specific("LANG_NAME_INTERNATIONAL", $code)
                ."/$lname)"
            ;
        }
    }
}

if (count($languages_in_use)) {
    $cur_lang_name = language_name($languages_in_use[0]);
} else {
    echo "foo";
    $cur_lang_name = language_name('en');
}

echo "<p>",
    tra(
        "This web site is available in several languages. The currently selected language is %1.",
        $cur_lang_name
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
        "Or you can select a language by clicking on one of the links.  This will send your browser a cookie; make sure your browser accepts cookies from our domain."
    ),
    "</p>"
;

start_table();
row1(tra("Language name (click to select)"));
row1("<a href=language_select.php?set_lang=auto>"
    .tra("Use browser language setting")
    ."</a>",
    1, "row1"
);
sort($languages);
$i = 0;
foreach ($languages as $language) {
    $name = language_name($language);
    row1(
        "<a href=\"language_select.php?set_lang=$language\">$name</a>",
        1, "row$i"
    );
    $i = 1-$i;
}
end_table();
echo "<p>",
    tra("Translations are done by volunteers.  If your native language is not here, %1you can provide a translation%2.",
    '<a href="http://boinc.berkeley.edu/trac/wiki/TranslateIntro">', '</a>'),
    "</p>"
;
page_tail();
?>
