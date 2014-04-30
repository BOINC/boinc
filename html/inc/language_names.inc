<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

$language_names = array(
    array("ar", "العربية", "Arabic"),
    array("be", "Беларускі", "Belarusian"),
    array("bg", "Български", "Bulgarian"),
    array("ca", "Català", "Catalan"),
    array("cs", "Čeština ", "Czech"),
    array("cy", "Cymraeg", "Welsh"),
    array("da", "Dansk", "Danish"),
    array("de", "Deutsch ", "German"),
    array("el", "﻿Ελληνικά", "Greek"),
    array("en", "English", "English"),
    array("es", "Espa&ntilde;ol", "Spanish"),
    array("sa", "فارسی", "Persian"),
    array("fi", "Suomi", "Finnish"),
    array("fr", "Fran&ccedil;ais", "French"),
    array("gl", "Galego", "Galician"),
    array("he", "עִבְרִית", "Hebrew"),
    array("hr", "Hrvatski", "Croatian"),
    array("hu", "Magyar", "Hungarian"),
    array("hu", "بهاس ملايو", "Malay"),
    array("it", "Italiano", "Italian"),
    array("ja", "日本語", "Japanese"),
    array("ko", "한국어", "Korean"),
    array("lt", "Lietuvių", "Lithuanian"),
    array("lv", "Latviešu", "Latvian"),
    array("nl", "Nederlands", "Dutch"),
    array("pl", "Polski", "Polish"),
    array("pt_BR", "Portugu&ecirc;s - Brasil", "Portuguese - Brazil"),
    array("pt_PT", "Portugu&ecirc;s", "Portuguese"),
    array("ro", "Română", "Romanian"),
    array("ru", "Русский", "Russian"),
    array("sk", "Slovenčina", "Slovakian"),
    array("sl", "Slovenščina", "Slovenian"),
    array("sv", "Svenska", "Swedish"),
    array("tr", "Türkçe", "Turkish"),
    array("zh_CN", "简体中文", "Chinese"),
    array("zh_TW", "繁體中文", "Chinese (Taiwan)"),
);

function language_select() {
    global $language_names;
    $supported_languages = get_supported_languages();
    $supported_languages[] = "en";
    $bd = tra("Browser default");
    echo "
        <option value=auto selected=\"selected\">-- language --
        <option value=auto>$bd
    ";
    if ($bd != "Browser default") echo " (Browser default)";
    foreach ($language_names as $lang) {
        if (!in_array($lang[0], $supported_languages)) continue;
        if ($lang[0] == "en") {
            echo "<option value=".$lang[0].">".$lang[1];
        } else {
            echo "<option value=".$lang[0].">".$lang[1]." (".$lang[2].")";
        }
    }
}

// show a menu of supported languages
//
function language_form() {
    echo "
        <table><tr><td>
        <form name=language method=get action=set_language.php>
        <select class=selectbox name=lang onchange=\"javascript: submit()\">
    ";
    language_select();
    echo "</select>
        </form>
        <script type=\"text/javascript\">
        document.language.lang.selectedIndex=0;
        </script>
        </td></tr></table>
    ";
}

?>
