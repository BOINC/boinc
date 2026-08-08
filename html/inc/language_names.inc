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
    array("az", "Azərbaycan", "Azerbaijani"),
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
    array("eu", "Basque", "Basque"),
    array("fi", "Suomi", "Finnish"),
    array("fr", "Fran&ccedil;ais", "French"),
    array("gl", "Galego", "Galician"),
    array("he", "עִבְרִית", "Hebrew"),
    array("hr", "Hrvatski", "Croatian"),
    array("hu", "Magyar", "Hungarian"),
    array("it_IT", "Italiano", "Italian"),
    array("ja", "日本語", "Japanese"),
    array("ko", "한국어", "Korean"),
    array("lt", "Lietuvių", "Lithuanian"),
    array("lv", "Latviešu", "Latvian"),
    array("ms", "بهاس ملايو", "Malay"),
    array("nb", "Norsk", "Norwegian"),
    array("nl", "Nederlands", "Dutch"),
    array("pl", "Polski", "Polish"),
    array("pt_BR", "Portugu&ecirc;s - Brasil", "Portuguese - Brazil"),
    array("pt_PT", "Portugu&ecirc;s", "Portuguese"),
    array("ro", "Română", "Romanian"),
    array("ru", "Русский", "Russian"),
    array("sa", "فارسی", "Persian"),
    array("sk", "Slovenčina", "Slovakian"),
    array("sl", "Slovenščina", "Slovenian"),
    array("sv", "Svenska", "Swedish"),
    array("tr", "Türkçe", "Turkish"),
    array("uk", "Українська", "Ukrainian"),
    array("zh_CN", "简体中文", "Chinese"),
    array("zh_TW", "繁體中文", "Chinese (Taiwan)"),
);

function language_select($cur_lang_name) {
    global $language_names;
    $supported_languages = get_supported_languages();
    $supported_languages[] = "en";
    $sel = $cur_lang_name?"":"selected";
    echo "
        <option $sel value=auto>Language: default
    ";
    foreach ($language_names as $lang) {
        if (!in_array($lang[0], $supported_languages)) continue;
        $sel = ($cur_lang_name == $lang[0])?"selected":"";
        if ($lang[0] == "en") {
            echo "<option $sel value=".$lang[0].">".$lang[1];
        } else {
            echo "<option $sel value=".$lang[0].">".$lang[1]." (".$lang[2].")";
        }
        echo "\n";
    }
}

// show a menu of supported languages
//
function language_form($cur_lang_name=null) {
    echo '
        <form name="language" method="get" action="set_language.php">
        <select class="selectbox form-control" name="lang" onchange="javascript: submit()">
    ';
    language_select($cur_lang_name);
    echo '</select>
        </form>
    ';
}

// given a language code, return a string of the form "(int-name/native-name)"
//
function language_desc($code) {
    global $language_names;
    if ($code == 'en') {
        return '(English)';
    }
    foreach ($language_names as $lang) {
        if ($code == $lang[0]) {
            return "'$code'".' ('.$lang[1].'/'.$lang[2].')';
        }
    }
    return "'$code'";
}

?>
