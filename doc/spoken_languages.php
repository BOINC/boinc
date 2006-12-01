<?php
$spoken_languages = array(
    'Arabic',
    'Awadhi',
    'Azerbaijani, South',
    'Bengali',
    'Bhojpuri',
    'Burmese',
    'Chinese, Gan',
    'Chinese, Hakka',
    'Chinese, Jinyu',
    'Chinese, Mandarin',
    'Chinese, Min Nan',
    'Chinese, Wu',
    'Chinese, Xiang',
    'Chinese, Yue (Cantonese)',
    'Dutch',
    'English',
    'French',
    'German',
    'Gujarati',
    'Hausa',
    'Hindi',
    'Italian',
    'Japanese',
    'Javanese',
    'Kannada',
    'Korean',
    'Maithili',
    'Malayalam',
    'Marathi',
    'Oriya',
    'Panjabi, Eastern',
    'Panjabi, Western',
    'Persian',
    'Polish',
    'Portuguese',
    'Romanian',
    'Russian',
    'Sunda',
    'Serbo-Croatian',
    'Sindi',
    'Spanish',
    'Tamil',
    'Telugu',
    'Thai',
    'Turkish',
    'Ukrainian',
    'Urdu',
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
