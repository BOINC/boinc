<?php

// Place your version in project_specific/project_specific_presf.inc

// Functions to display and edit project-specific prefs go here

// The code here is a sample.  Projects must supply their own.

function option($name, $val) {
    if ($name == $val) {
        $x = "selected";
    } else {
        $x = "";
    }
    return "<option name='$name' $x>$name\n";
}

function project_specific_prefs_default() {
    return "<color_scheme>Tahiti Sunset</color_scheme>\n";
}

// given struct, show form for editing
//
function project_specific_prefs_edit($prefs) {
    $x = $prefs->color_scheme;
    $y = "<select name=color_scheme>
            ".option("Tahiti Sunset", $x)
            .option("Desert Sands", $x)."
        </select>
    ";
    row2("Color scheme", $y);
}

// Return XML version of project-specific prefs
//
function project_specific_prefs_parse_form() {
    $color_scheme = $_GET["color_scheme"];
    return "<color_scheme>$color_scheme</color_scheme>\n";
}

// show non-editable version of prefs
//
function project_specific_prefs_show($prefs) {
    if ($prefs->color_scheme) {
        $x = $prefs->color_scheme;
    } else {
        $x = "None selected";
    }
    row2("Color scheme", $x);
}

// parse XML, fill in struct
//
function project_specific_prefs_parse($prefs_xml) {
    $prefs->color_scheme = parse_element($prefs_xml, "<color_scheme>");
    return $prefs;
}
