<?php

// Functions to display and edit project-specific prefs go here

// The code here is a sample.  Projects must supply their own.

function option($name, $val) {
    if ($name == $val) {
        $x = "selected";
    } else {
        $x = "";
    }
    echo "<option name='$name' $x>$name\n";
}

function project_specific_prefs_default() {
    return "<color_scheme>Tahiti Sunset</color_scheme>\n";
}

// given struct, show form for editing
//
function project_specific_prefs_edit($prefs) {
    $x = $prefs->color_scheme;
    echo "<tr>\n
        <td align=right>Color scheme:</td>\n
        <td>\n
            <select name=color_scheme>\n";
                option("Tahiti Sunset", $x);
                option("Desert Sands", $x);
    echo "
            </select>\n
        </td>\n
        </tr>";
}

// QUERY_STRING has already been parsed into variables.
// Return XML version of project-specific prefs
//
function project_specific_prefs_parse_form() {
    parse_str(getenv("QUERY_STRING"));
    return "<color_scheme>$color_scheme</color_scheme>";
}

// show non-editable version of prefs
//
function project_specific_prefs_show($prefs) {
    echo "<tr>\n
        <td>Color scheme:</td>\n
        <td>";
    if ($prefs->color_scheme) {
        echo $prefs->color_scheme;
    } else {
        echo "None selected";
    }
    echo "</td></tr>\n";
}

// parse XML, fill in struct
//
function project_specific_prefs_parse($prefs_xml) {
    $prefs->color_scheme = parse_element($prefs_xml, "<color_scheme>");
    return $prefs;
}
