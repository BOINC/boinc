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

$lang_language_dir = "../languages/";
$lang_translations_dir = "translations/";
$lang_prj_translations_dir = "project_specific_translations/";
$lang_compiled_dir = "compiled/";
$lang_log_level = 1;

// Get a list of compiled languages by scanning the compiled/ dir
// @returns A list of languages that have been compiled
//
function get_supported_languages() {
    global $lang_language_dir, $lang_compiled_dir;
    $list = array();
    if (!is_dir($lang_language_dir.$lang_compiled_dir)) {
        echo "\"".$lang_language_dir.$lang_compiled_dir."\" is not a directory. Please consult the documentation for correctly setting up the translation system.";
        exit;
    }
    $dh = opendir($lang_language_dir.$lang_compiled_dir);
    if (!$dh) die("can't open language dir");

    while ($file = readdir($dh)) {
        if (substr($file, -7) != ".po.inc") continue;
        if (is_numeric(substr($file, 0, 5))) continue;
        $list[] = substr($file, 0, -7);
    }
    return $list;
}

// generate PHP files defining translation arrays.
// For example, the file "ca.po.inc" would contain entries of the form
// $language_lookup_array["ca"]["Default"] = "Defecte";
//
// Append to these files if they already exist
// (this may get done for both generic and project-specific translations)
//
// @param langdir The language base directory
// @param transdir The location of the .po files to compile relative to langdir
// @param compdir The output location relative to langdir
//
function build_translation_array_files($langdir, $transdir, $compdir) {

    // Run through each language and compile their lookup arrays.
    //
    if (!is_dir($langdir.$transdir)) {
        //debug("$info_dir not found or is not a directory");
    }
    $dh = opendir($langdir.$transdir);
    if (!$dh) die("can't open translation dir");
    while (($file = readdir($dh)) !== false) {
        if ($file==".." || $file==".") {
            continue;
        }
        // only do files ending in .po
        if (substr($file,-3) != ".po"){
            //debug("File $file with unknown extension found in $info_dir");
            continue;
        }
        language_log(
            "-------------Compiling $transdir$file------------", 0
        );
        $language = parse_po_file($langdir.$transdir.$file);
        if (!$language){
            language_log(
                "WARNING: Could not parse language ".$file
            );
            continue;
        }
        $path = $langdir.$compdir.$file.".inc";
        if (file_exists($path)) {
            $fh = fopen($path, "a");
        } else {
            $fh = fopen($path, "w");
            fwrite($fh, "<?php\n");
        }
        if (!$fh) {
            language_log(
                "ERROR: could not access $langdir $compdir - please check permissions", 2
            );
            exit;
        }
        foreach ($language as $key => $value){
            if ($value !== "") {
                // Skip if the msgstr is empty
                fwrite($fh, "\$language_lookup_array[\"".str_replace("\"", "\\\"", substr($file,0,-3))."\"][\"".$key."\"] = \"".$value."\";\n");
            }
        }
        // don't write \?\> - may append

        fclose($fh);
    }
    closedir($dh);
}

// Parses a gettext .po-file into an associative PHP array.
// @param file The file to parse
// checking for inconsistencies if needed.
//
function parse_po_file($file) {
    $translation_file = file($file);
    $first_entry = true;
    $current_token_text="";
    $current_token ="";
    $parsing_token = false;
    $parsing_text = false;
    $size = sizeof($translation_file);
    $output = array();
    for ($i=0; $i<$size; $i++){
        $entry = trim($translation_file[$i]);
        //echo "line $i: $entry\n";
        if (substr($entry, 0, 1)=="#") {
            continue;
        } elseif (strpos($entry, "msgid") !== false) {
            if (!$first_entry){
                //If this is not the first, save the previous entry
                $output[$current_token]=$current_token_text;
            }
            $current_token = get_po_line($entry, $file);
            $current_token_text="";
            $parsing_token = true;
            $parsing_text = false;
            $first_entry=false;
        } elseif (strpos($entry, "msgstr") !== false) {
            $current_token_text = get_po_line($entry, $file);
            $parsing_token = false;
            $parsing_text = true;
        } elseif ($parsing_token) {
            $current_token .= get_po_line($entry, $file);
        } elseif ($parsing_text) {
            $current_token_text .= get_po_line($entry, $file);
        }
    }

    // Get the last token
    //
    if ($current_token && $current_token_text){
        $output[$current_token] = $current_token_text;
    }
    return $output;
}


// Returns the contents of a line (ie removes "" from start and end)
//
function get_po_line($line, $file) {
    $start = strpos($line, '"')+1;
    $stop = strrpos($line, '"');
    $x = substr($line, $start, $stop-$start);
    $n = preg_match("/[^\\\\]\"/", $x);
    if ($n) {
        echo "ERROR - MISMATCHED QUOTES IN $file: $line\n";
        return "";
    }
    return $x;
}

//////////  EVERYTHING BEFORE HERE IS FOR ops/update_translations.php,
//////////  AND SHOULD BE MOVED TO A SEPARATE FILE

// Translate string
//
function tra($text /* ...arglist... */) {
    global $language_lookup_array, $languages_in_use;

    // Find the string in the user's language
    //
    foreach ($languages_in_use as $language){
        if (isset($language_lookup_array[$language][$text])) {
            $text = $language_lookup_array[$language][$text];
            break;
        } else if ($language=="en"){
            // This language is defined in the code and is always available
            break;
        }
    }

    // Replace relevant substrings with given arguments.
    // Use strtr to avoid problems if an argument contains %n.
    $replacements = array();
    for ($i=1; $i<func_num_args(); $i++){
        $replacements["%".$i] = func_get_arg($i);
    }

    return strtr($text, $replacements);
}

function tr_specific($text, $language) {
    global $lang_language_dir, $lang_compiled_dir, $language_lookup_array;
    $file_name = $lang_language_dir.$lang_compiled_dir.$language.".po.inc";
    if (file_exists($file_name)) {
        require_once($file_name);
        $text = $language_lookup_array[$language][$text];
    }
    return $text;
}

function language_log($message, $loglevel=0) {
    global $lang_log_level;
    $msg = "";
    if ($loglevel==0) $msg = "[ Debug    ]";
    if ($loglevel==1) $msg = "[ Warning  ]";
    if ($loglevel==2) $msg = "[ CRITICAL ]";

    if ($loglevel >= $lang_log_level){
        echo gmdate("Y-m-d H:i:s", time())." ".$msg." ".$message."\n";
    }
}

// Make a list of languages which the user prefers
// (by looking at cookies and browser settings)
// cookies have highest priority.

if (isset($_COOKIE['lang'])){
    $language_string = $_COOKIE['lang'].",";
} else {
    $language_string = '';
}
if (isset($_SERVER["HTTP_ACCEPT_LANGUAGE"])) {
    $language_string .= strtolower($_SERVER["HTTP_ACCEPT_LANGUAGE"]);
}

// Find out which language to use by iterating through list
// The list is comma-separated, so split it into an array of the following type:
//        Array (
//    [0] => da
//    [1] => en-us;q=0.7
//    [2] => en;q=0.3
//  )

$client_languages = explode(",", $language_string);

// A language is either defined as primary-secondary or primary.
// It can also have a quality attribute set,
// which orders the languages in a user preferred ordering.
// Since this is usally the same order as the array indices
// we just ignore this attribute (TODO: don't ignore this attribute)
// A missing quality attribute means q=1

$languages_in_use = array();

// Loop over languages that the client requests
//
$size = sizeof($client_languages);
for ($i=0; $i<$size; $i++) {
    if ((strlen($client_languages[$i])>2)
        && (substr($client_languages[$i], 2, 1) == "_" || substr($client_languages[$i], 2, 1) == "-")
    ){
        // If this is defined as primary-secondary, represent it as xx_YY
        //
        $language = substr(
            $client_languages[$i], 0, 2)."_".strtoupper(substr($client_languages[$i], 3, 2)
        );

        // And also check for the primary language
        //
        $language2 = substr($client_languages[$i], 0, 2);
    } else {
        // else just use xx
        //
        $language = substr($client_languages[$i], 0, 2);
        $language2 = null;
    }

    // if main language is english, look no further
    //
    if ((count($languages_in_use)==0) && ($language == 'en' || $language2 == 'en')) {
        break;
    }

    // If we have a translation for the language, include it
    //
    $file_name = $lang_language_dir.$lang_compiled_dir.$language.".po.inc";
    if (file_exists($file_name)) {
        if (!in_array($language, $languages_in_use)){
            require_once($file_name);
            $languages_in_use[] = $language;
        }
    }
    if ($language2) {
        $file_name = $lang_language_dir.$lang_compiled_dir.$language2.".po.inc";
        if (file_exists($file_name)) {
            if (!in_array($language2, $languages_in_use)){
                require_once($file_name);
                $languages_in_use[] = $language2;
            }
        }
    }
}

$GLOBALS['languages_in_use'] = $languages_in_use;   // for Drupal

?>
