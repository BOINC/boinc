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

// support for page-level translation
// Some of this should be merged with translation.inc

function get_lang_list() {
    if (isset($_COOKIE['lang'])){
        $language_string = $_COOKIE['lang'].",";
    } else {
        $language_string = '';
    }
    if (isset($_SERVER["HTTP_ACCEPT_LANGUAGE"])) {
        $language_string .= strtolower($_SERVER["HTTP_ACCEPT_LANGUAGE"]);
    }
    $client_languages = explode(",",$language_string);

    $lang_list = array();
    for ($i=0; $i<sizeof($client_languages); $i++) {
        if ((strlen($client_languages[$i])>2)
            && (substr($client_languages[$i],2,1)=="_" || substr($client_languages[$i],2,1)=="-"))
        {
            // If this is defined as primary-secondary, represent it as xx_YY
            //
            $language = substr(
                $client_languages[$i],0,2)."_".strtoupper(substr($client_languages[$i],3,2)
            );
            $lang_list[] = $language;

            // And also check for the primary language
            //
            $language = substr($client_languages[$i],0,2);
            $lang_list[] = $language;
        } else {
            // else just use xx
            //
            $language = substr($client_languages[$i],0,2);
            $lang_list[] = $language;
        }
    }
    return $lang_list;
}

function find_translation($file) {
    $lang_list = get_lang_list();
    foreach ($lang_list as $lang) {
        $path = "language_dirs/$lang/$file";
        if (file_exists($path)) {
            readfile($path);
            exit();
        }
    }
}


?>
