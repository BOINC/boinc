<?php

// see http://en.wikipedia.org/wiki/List_of_languages_by_number_of_native_speakers
$spoken_languages = array(
    'Arabic',
    'Awadhi',
    'Azeri',
    'Bengali',
    'Bhojpuri',
    'Burmese',
    'Catalan',
    'Chinese, Gan',
    'Chinese, Hakka',
    'Chinese, Jinyu',
    'Chinese, Mandarin',
    'Chinese, Min Nan',
    'Chinese, Wu',
    'Chinese, Xiang',
    'Chinese, Yue (Cantonese)',
    'Czech',
    'Dutch',
    'English',
    'French',
    'German',
    'Greek',
    'Gujarati',
    'Hausa',
    'Hindi',
    'Hungarian/Magyar',
    'Italian',
    'Japanese',
    'Javanese',
    'Kannada',
    'Korean',
    'Lithuanian',
    'Maithili',
    'Malayalam',
    'Marathi',
    'Oriya',
    'Punjabi',
    'Persian',
    'Polish',
    'Portuguese',
    'Romanian',
    'Russian',
    'Sundanese',
    'Serbo-Croatian',
    'Sindi',
    'Spanish',
    'Tagalog',
    'Tamil',
    'Telugu',
    'Thai',
    'Turkish',
    'Ukrainian',
    'Urdu',
    'Uzbek',
    'Vietnamese',
    'Yoruba',
);

function is_spoken_language($lang) {
    global $spoken_languages;
    if (!$lang) return true;
    return in_array($lang, $spoken_languages);
}

function spoken_language_list($name, $val) {
    global $spoken_languages;
    $x = "<select name=$name>\n";
    if ($val) {
        $x .= "<option value=\"\">---\n";
    } else {
        $x .= "<option value=\"\" selected >---\n";
    }

    foreach ($spoken_languages as $lang) {
        if ($lang == $val) {
            $x .= "<option value=\"$lang\" selected>$lang\n";
        } else {
            $x .= "<option value=\"$lang\">$lang\n";
        }
    }
    $x .= "</select>\n";
    return $x;
}

?>
