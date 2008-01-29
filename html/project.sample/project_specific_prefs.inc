<?php

// Sample code for project-specific preferences.
// These prefs may include:
//
// - preferences for your application graphics
// - application selection (i.e. if you have muliple apps,
//   let user choose among them)

// These prefs are represented in three ways:
// - as an XML document (stored in the DB in this form)
// - as a PHP structure
// - as a set of HTTP GET form variables

// This file exports the following functions
// (called from html/inc/prefs.inc):
//
// project_specific_prefs_default()
//      Returns XML for default preferences
// project_specific_prefs_parse($xml)
//      Parse prefs as XML, return prefs as PHP structure
// project_specific_prefs_show($prefs, $columns=false)
//      Show prefs as HTML (non-editable)
// project_specific_prefs_edit($prefs, $error)
//      Show prefs as HTML, editable.
//      $error is a struct indicating which values were erroneous
//      (value X is erroneous if $error->X is set)
// project_specific_prefs_parse_form(&$error)
//      Parse form variables into XML, and return it.
//      Also error-check values, and return errors in $errors->*

// Place your version in html/project_specific/project_specific_prefs.inc

function option($name, $val) {
    $x = ($name == $val) ?  "selected" : "";
    return "<option name='$name' $x>$name\n";
}

define('COLOR_DESC', 'Color scheme for graphics');
define('FRAME_RATE_DESC', 'Max frames/second for graphics<br><span class=note>0 or blank if no limit</span>');
define("MAX_GFX_CPU_PCT_DESC", "Maximum CPU % for graphics<br><span class=note>0 ... 100</span>");
define('APP_SELECT_DESC', 'Run only the selected applications');

// The following array is for app filtering; uncomment if you want it.
// Note: in this implementation, if a user selects all apps,
// no <app_id> elements are included in their prefs,
// which means that if the project adds a new app such users will run it also.
//
if (false) {
    $app_array = array(
        array(1, "Application one"),
        array(2, "Application two"),
    );
} else {
    $app_array = null;
}

function selected_app_text($prefs) {
    global $app_array;
    if (isset($prefs->app_ids)) {
        $x = "";
        foreach ($app_array as $app) {
            $app_id = $app[0];
            $app_name = $app[1];
            if (in_array($app_id, $prefs->app_ids)) {
                $x .= "$app_name: yes<br>";
            } else {
                $x .= "$app_name: no<br>";
            }
        }
    } else {
        $x = "(all applications)";
    }
    return $x;
}

function project_specific_prefs_default() {
    return "
        <color_scheme>Tahiti Sunset</color_scheme>
        <max_frames_sec>30</max_frames_sec>
        <max_gfx_cpu_pct>20</max_gfx_cpu_pct>
    ";
}

function project_specific_prefs_edit($prefs, $error=false) {
    global $app_array;
    $x = $prefs->color_scheme;
    $y = "<select name=color_scheme>
            ".option("Tahiti Sunset", $x)
            .option("Desert Sands", $x)."
        </select>
    ";
    row2(COLOR_DESC, $y);

    $y = "<input size=5 name=max_frames_sec value='$prefs->max_frames_sec'>";
    row2(FRAME_RATE_DESC, $y, isset($error->max_frames_sec));
    $y = "<input size=5 name=max_gfx_cpu_pct value='$prefs->max_gfx_cpu_pct'>";
    row2(MAX_GFX_CPU_PCT_DESC, $y, isset($error->max_gfx_cpu_pct));
    if ($app_array) {
        $x = "";
        foreach ($app_array as $app) {
            $app_id = $app[0];
            $app_name = $app[1];
            if (isset($prefs->app_ids)) {
                $present = in_array($app_id, $prefs->app_ids);
            } else {
                $present = true;
            }
            $checked = $present?"checked":"";
            $x .= "<input type=checkbox name=app_id_$app_id $checked> $app_name<br>";
        }
        row2(APP_SELECT_DESC, $x);
    }
}

function project_specific_prefs_parse_form(&$error) {
    global $app_array;
    $color_scheme = $_GET["color_scheme"];
    $max_frames_sec = $_GET["max_frames_sec"];
    $max_gfx_cpu_pct = $_GET["max_gfx_cpu_pct"];

    // Error-check preferences
    //
    if (!verify_numeric($max_frames_sec, 0)) $error->max_frames_sec = true;
    if (!verify_numeric($max_gfx_cpu_pct, 0, 100)) $error->max_gfx_cpu_pct = true;

    $x = "<color_scheme>$color_scheme</color_scheme>
        <max_frames_sec>$max_frames_sec</max_frames_sec>
        <max_gfx_cpu_pct>$max_gfx_cpu_pct</max_gfx_cpu_pct>
";

    if ($app_array) {
        $y = "";
        $some_unchecked = false;
        foreach ($app_array as $app) {
            $app_id = $app[0];
            $present = isset($_GET["app_id_$app_id"]);
            if ($present) {
                $x .= "<app_id>$app_id</app_id>\n";
            } else {
                $some_unchecked = true;
            }
        }
        if ($some_unchecked) {
            $x .= $y;
        }
    }

    return $x;
}

function project_specific_prefs_show($prefs, $columns=false) {
    global $app_array;
    if ($columns) {
        row_defs(COLOR_DESC,"color_scheme", "", "", $prefs);
        row_defs(FRAME_RATE_DESC, "max_frames_sec", "", "", $prefs);
        row_defs(MAX_GFX_CPU_PCT_DESC, "max_gfx_cpu_pct", "", "", $prefs);
        if ($app_array) {
            $prefs->app_id_text = selected_app_text($prefs);
            if ($prefs->home) $prefs->home->app_id_text = selected_app_text($prefs->home);
            if ($prefs->school) $prefs->school->app_id_text = selected_app_text($prefs->school);
            if ($prefs->work) $prefs->work->app_id_text = selected_app_text($prefs->work);
            row_defs(APP_SELECT_DESC, "app_id_text", "", "", $prefs);
        }
    } else {
        row2(COLOR_DESC, $prefs->color_scheme);
        row2(FRAME_RATE_DESC, $prefs->max_frames_sec);
        row2(MAX_GFX_CPU_PCT_DESC, $prefs->max_gfx_cpu_pct);
        if ($app_array) {
            row2(APP_SELECT_DESC, selected_app_text($prefs));
        }
    }
}

function project_specific_prefs_parse($prefs_xml) {
    $prefs->color_scheme = parse_element($prefs_xml, "<color_scheme>");
    $prefs->max_frames_sec = parse_element($prefs_xml, "<max_frames_sec>");
    $prefs->max_gfx_cpu_pct = parse_element($prefs_xml, "<max_gfx_cpu_pct>");
    $cursor = 0;
    while ($thisxml = parse_next_element($prefs_xml, "<app_id>", $cursor)) {
        if (is_numeric($thisxml)) {
            $n = (int) $thisxml;
            $prefs->app_ids[] = $n;
        }
    }
    return $prefs;
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
