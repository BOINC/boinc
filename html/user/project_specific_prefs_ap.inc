<?php

// Functions to display and edit project-specific prefs
// (Astropulse version)
// The prefs are as follows:
// <color_scheme>
//      A few presets, plus "Custom" to control other parameters
// <start_hue>
//      Starting hue (0..1) for graph
// <hue_change>
//      Delta hue for graph (+ or -)
// <grow_time>
//      Time to grow graph, back to front
// <hold_time>
//      Time to hold completed graph
// <two_dim/>
//      If present, draw 2-D graph
// <graph_alpha>
//      transparency of graph (0..1)
// <app_file>
//     <open_name>background</open_name>
//     <url>...</url>
// </app_file>

function project_specific_prefs_default() {
    return "<color_scheme>Tahiti Sunset</color_scheme>\n";
}

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
    echo "<tr>
        <td align=right>Graphics preferences:
        <br><font size=-1>Select Custom to edit individual parameters</font>
        </td><td>
        <select name=color_scheme>
        ";
    option("Tahiti Sunset", $x);
    option("Desert Sands", $x);
    option("Underwater", $x);
    option("Custom", $x);
    echo "</select></td></tr>\n";
    if ($prefs->color_scheme == "Custom") {
        $sel2 = ($prefs->two_dim?"checked":"");
        $sel3 = ($prefs->two_dim?"":"checked");
        echo "
            <tr><td align=right>Start hue</td>
            <td><input name=start_hue value='$prefs->start_hue'></td></tr>
            <tr><td align=right>Hue change</td>
            <td><input name=hue_change value='$prefs->hue_change'></td></tr>
            <tr><td align=right>Grow time</td>
            <td><input name=grow_time value='$prefs->grow_time'></td></tr>
            <tr><td align=right>Hold time</td>
            <td><input name=hold_time value='$prefs->hold_time'></td></tr>
            <tr><td align=right>Graph dimension</td>
            <td><input type=radio name=two_dim value=yes $sel2> 2-D
            <input type=radio name=two_dim value=no $sel3> 3-D</td></tr>
            <tr><td align=right>Graph transparency</td>
            <td><input name=graph_alpha value='$prefs->graph_alpha'></td></tr>
        ";
    }
    echo "<tr>
        <td align=right>URL of background (PPM format)</td>
        <td><input size=60 name=background value='$prefs->background'></td>
        </tr>
    ";
}

// Return XML version of project-specific prefs from form vars
//
function project_specific_prefs_parse_form() {
    $color_scheme = $_GET["color_scheme"];
    $start_hue = $_GET["start_hue"];
    $hue_change = $_GET["hue_change"];
    $grow_time = $_GET["grow_time"];
    $hold_time = $_GET["hold_time"];
    $graph_alpha = $_GET["graph_alpa"];
    $two_dim = $_GET["two_dim"];
    $background = $_GET["background"];

    switch($color_scheme) {
    case "Tahiti Sunset":
        $start_hue = 0.5;
        $hue_change = 0.5;
        $grow_time = 10;
        $hold_time = 5;
        $graph_alpha = 0.7;
        $two_dim = "no";
        break;
    case "Desert Sands":
        $start_hue = 0.0;
        $hue_change = 0.2;
        $grow_time = 10;
        $hold_time = 5;
        $graph_alpha = 0.7;
        $two_dim = "no";
        break;
    case "Underwater":
        $start_hue = 0.2;
        $hue_change = 0.4;
        $grow_time = 10;
        $hold_time = 5;
        $graph_alpha = 0.7;
        $two_dim = "no";
        break;
    case "Custom":
        if ($start_hue == null) $start_hue = 0;
        if ($hue_change == null) $hue_change = 1;
        if ($grow_time == null) $grow_time = 10;
        if ($hold_time == null) $hold_time = 5;
        if ($graph_alpha == null) $graph_alpha = 0.7;
        if ($two_dim == null) $two_dim = "no";
        break;
    }
    $x = "<color_scheme>$color_scheme</color_scheme>
        <start_hue>$start_hue</start_hue>
        <hue_change>$hue_change</hue_change>
        <grow_time>$grow_time</grow_time>
        <hold_time>$hold_time</hold_time>
        <graph_alpha>$graph_alpha</graph_alpha>
    ";
    if ($two_dim == "yes") $x = $x."<two_dim/>\n";
    if (strlen($background)) {
        $x = $x."<app_file>
        	<timestamp>".time()."</timestamp>
            <url>$background</url>
            <open_name>background</open_name>
            </app_file>
        ";
    }
    return $x;
}

// show non-editable version of prefs
//
function project_specific_prefs_show($prefs) {
    row2("Color scheme", $prefs->color_scheme);
    if ($prefs->color_scheme == "Custom") {
        $dim = $prefs->two_dim?"2":"3";
        row2("Start hue", $prefs->start_hue);
        row2("Hue change", $prefs->hue_change);
        row2("Grow time", $prefs->grow_time);
        row2("Hold time", $prefs->hold_time);
        row2("Graph alpha", $prefs->graph_alpha);
        row2("Graph dimension", $dim);
    }
    if (strlen($prefs->background)) {
        row2("Background", $prefs->background);
    } else {
        row2("Background", "none");
    }
}

// parse XML, fill in struct
//
function project_specific_prefs_parse($prefs_xml) {
    $prefs->color_scheme = parse_element($prefs_xml, "<color_scheme>");
    $prefs->start_hue = parse_element($prefs_xml, "<start_hue>");
    $prefs->hue_change = parse_element($prefs_xml, "<hue_change>");
    $prefs->grow_time = parse_element($prefs_xml, "<grow_time>");
    $prefs->hold_time = parse_element($prefs_xml, "<hold_time>");
    $prefs->graph_alpha = parse_element($prefs_xml, "<graph_alpha>");
    $prefs->two_dim = parse_element($prefs_xml, "<two_dim>");
    $prefs->background = parse_element($prefs_xml, "<url>");
    return $prefs;
}
