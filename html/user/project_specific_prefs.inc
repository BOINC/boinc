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
        <td><b>Color scheme:</b></td>\n
        <td>$prefs->color_scheme</td>\n
        </tr>\n";
}

// parse XML, fill in struct
//
function project_specific_prefs_parse($prefs) {
    $xml = $prefs->project_specific;
    $prefs->color_scheme = parse_element($xml, "<color_scheme>");
    return $prefs;
}
