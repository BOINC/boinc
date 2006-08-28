<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
// Place your version in project_specific/project_specific_prefs.inc

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
    return "
        <color_scheme>Tahiti Sunset</color_scheme>
        <refresh_rate>100</refresh_rate>
    ";
}

// given struct, show form for editing
// $error is a struct indicating which values were erroneous
// (value X is erroneous if $error->X is set)
//
function project_specific_prefs_edit($prefs, $error=false) {
    $x = $prefs->color_scheme;
    $y = "<select name=color_scheme>
            ".option("Tahiti Sunset", $x)
            .option("Desert Sands", $x)."
        </select>
    ";
    row2("Color scheme", $y);

    $y = "<input size=5 name=refresh_rate value='$prefs->refresh_rate'> Hertz ";
    row2("Refresh Rate", $y, isset($error->refresh_rate));
}

// Parse form vars, return XML version of project-specific prefs
// Also set $error, store the error values (see above)
//
function project_specific_prefs_parse_form(&$error) {
    $color_scheme = $_GET["color_scheme"];
    $refresh_rate = $_GET["refresh_rate"];

    // add and modify this line for other user
    // editable values that should be validated
    //
    if (!verify_numeric($refresh_rate, 0)) $error->refresh_rate = true;

    // Please look at util.inc for further information regarding:
    // function verify_numeric(&$value, $low, $high = false)

    return "<color_scheme>$color_scheme</color_scheme>
        <refresh_rate>$refresh_rate</refresh_rate>
    ";
}

// show non-editable version of prefs
//
function project_specific_prefs_show($prefs, $columns=false) {
    // Please add your prefs-values for both views!!
    //
    if ($columns) {
        // This is used if columns-view is enabled
        row_defs("Color scheme","color_scheme", "", "", $prefs);
        row_defs("Refresh rate", "refresh_rate", "Hertz", "", $prefs);
    } else {
        // This is used if normal-view is enabled
        row2("Color scheme", $prefs->color_scheme);
        row2("Refresh rate", $prefs->refresh_rate." Hertz");
    }
}

// parse XML, fill in struct
//
function project_specific_prefs_parse($prefs_xml) {
    $prefs->color_scheme = parse_element($prefs_xml, "<color_scheme>");
    $prefs->refresh_rate = parse_element($prefs_xml, "<refresh_rate>");
    return $prefs;
}
